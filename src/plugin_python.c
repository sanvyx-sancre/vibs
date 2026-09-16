#include "plugin.h"
#include <Python.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

static int python_ready;
static int python_plugin_count;
static char python_status[256] = "Python plugins not initialized";

// make loader state visible through a normal editor command
static int python_status_command(const char *args, char *message, size_t message_size, void *userdata) {
    (void)args;
    (void)userdata;
    if (message && message_size > 0) snprintf(message, message_size, "%s", python_status);
    return 1;
}

// map python event names to the internal event enum
static int event_from_name(const char *name, vibs_event_t *event) {
    static const char *names[] = {
        "startup", "file_open", "before_save", "after_save",
        "text_changed", "cursor_move", "shutdown"
    };
    for (int i = 0; i <= VIBS_EVENT_SHUTDOWN; i++) {
        if (strcmp(name, names[i]) == 0) {
            *event = (vibs_event_t)i;
            return 1;
        }
    }
    return 0;
}

// call a python function when a registered command runs
static int python_command_callback(const char *args, char *message, size_t message_size, void *userdata) {
    PyObject *callback = userdata;
    PyObject *result = PyObject_CallFunction(callback, "(s)", args);
    if (!result) {
        PyErr_Print();
        snprintf(message, message_size, "Python plugin command failed");
        return 0;
    }
    if (message && message_size > 0 && PyUnicode_Check(result)) {
        const char *text = PyUnicode_AsUTF8(result);
        if (text) snprintf(message, message_size, "%s", text);
    }
    Py_DECREF(result);
    return 1;
}

// call a python function when an editor event fires
static void python_event_callback(vibs_event_t event, void *userdata) {
    (void)event;
    PyObject *callback = userdata;
    PyObject *result = PyObject_CallObject(callback, NULL);
    if (!result) PyErr_Print();
    else Py_DECREF(result);
}

// expose command registration to python
static PyObject *py_command(PyObject *self, PyObject *args) {
    (void)self;
    const char *name;
    PyObject *callback;
    if (!PyArg_ParseTuple(args, "sO:command", &name, &callback)) return NULL;
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "command callback must be callable");
        return NULL;
    }
    Py_INCREF(callback);
    if (!vibs_register_command(name, python_command_callback, callback)) {
        Py_DECREF(callback);
        PyErr_SetString(PyExc_ValueError, "could not register command");
        return NULL;
    }
    Py_RETURN_NONE;
}

// expose event registration to python
static PyObject *py_on(PyObject *self, PyObject *args) {
    (void)self;
    const char *name;
    PyObject *callback;
    vibs_event_t event;
    if (!PyArg_ParseTuple(args, "sO:on", &name, &callback)) return NULL;
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "event callback must be callable");
        return NULL;
    }
    if (!event_from_name(name, &event)) {
        PyErr_SetString(PyExc_ValueError, "unknown vibs event");
        return NULL;
    }
    Py_INCREF(callback);
    if (!vibs_register_event(event, python_event_callback, callback)) {
        Py_DECREF(callback);
        PyErr_SetString(PyExc_ValueError, "could not register event");
        return NULL;
    }
    Py_RETURN_NONE;
}

// expose basic buffer and cursor access to python
static PyObject *py_lines(PyObject *self, PyObject *args) {
    (void)self;
    (void)args;
    PyObject *result = PyList_New(0);
    if (!result) return NULL;
    for (size_t i = 0; i < vibs_line_count(); i++) {
        const char *line = vibs_get_line(i);
        if (!line) break;
        PyObject *value = PyUnicode_FromString(line);
        if (!value || PyList_Append(result, value) < 0) {
            Py_XDECREF(value);
            Py_DECREF(result);
            return NULL;
        }
        Py_DECREF(value);
    }
    return result;
}

static PyObject *py_get_line(PyObject *self, PyObject *args) {
    (void)self;
    Py_ssize_t line;
    if (!PyArg_ParseTuple(args, "n:get_line", &line)) return NULL;
    const char *text = vibs_get_line((size_t)line);
    if (!text) {
        PyErr_SetString(PyExc_IndexError, "line is outside the buffer");
        return NULL;
    }
    return PyUnicode_FromString(text);
}

static PyObject *py_set_line(PyObject *self, PyObject *args) {
    (void)self;
    Py_ssize_t line;
    const char *text;
    if (!PyArg_ParseTuple(args, "ns:set_line", &line, &text)) return NULL;
    if (!vibs_set_line((size_t)line, text)) {
        PyErr_SetString(PyExc_ValueError, "could not replace line");
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *py_cursor(PyObject *self, PyObject *args) {
    (void)self;
    (void)args;
    int line, column;
    vibs_get_cursor(&line, &column);
    return Py_BuildValue("(ii)", line, column);
}

static PyObject *py_set_cursor(PyObject *self, PyObject *args) {
    (void)self;
    int line, column;
    if (!PyArg_ParseTuple(args, "ii:set_cursor", &line, &column)) return NULL;
    if (!vibs_set_cursor(line, column)) {
        PyErr_SetString(PyExc_ValueError, "invalid cursor position");
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *py_current_file(PyObject *self, PyObject *args) {
    (void)self;
    (void)args;
    const char *path = vibs_current_file();
    if (!path) Py_RETURN_NONE;
    return PyUnicode_FromString(path);
}

// define the small python module used by plugins
static PyMethodDef vibs_methods[] = {
    {"command", py_command, METH_VARARGS, "Register a colon command."},
    {"on", py_on, METH_VARARGS, "Register an editor event callback."},
    {"lines", py_lines, METH_NOARGS, "Return the non-empty buffer lines."},
    {"get_line", py_get_line, METH_VARARGS, "Read a buffer line."},
    {"set_line", py_set_line, METH_VARARGS, "Replace a buffer line."},
    {"cursor", py_cursor, METH_NOARGS, "Return (line, column)."},
    {"set_cursor", py_set_cursor, METH_VARARGS, "Set (line, column)."},
    {"current_file", py_current_file, METH_NOARGS, "Return the current filename."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef vibs_module = {
    PyModuleDef_HEAD_INIT, "vibs", "vibs editor plugin API", -1, vibs_methods,
    NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit_vibs(void) {
    return PyModule_Create(&vibs_module);
}

// initialize python and load user plugins from one directory
int vibs_python_load(const char *plugin_dir) {
    if (python_ready) return 1;
    if (PyImport_AppendInittab("vibs", &PyInit_vibs) == -1) {
        snprintf(python_status, sizeof(python_status), "Python plugin module registration failed");
        return 0;
    }
    Py_Initialize();
    python_ready = 1;
    vibs_register_command("PluginStatus", python_status_command, NULL);

    DIR *directory = opendir(plugin_dir);
    if (!directory) {
        snprintf(python_status, sizeof(python_status), "Python ready; plugin directory not found: %s", plugin_dir);
        return 1;
    }
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        size_t name_length = strlen(entry->d_name);
        if (name_length < 4 || strcmp(entry->d_name + name_length - 3, ".py") != 0) continue;
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", plugin_dir, entry->d_name);
        FILE *file = fopen(path, "r");
        if (!file) continue;
        int result = PyRun_SimpleFile(file, path);
        fclose(file);
        if (result != 0 || PyErr_Occurred()) {
            fprintf(stderr, "vibs: failed to load plugin %s\n", path);
            PyErr_Print();
            continue;
        }
        python_plugin_count++;
    }
    closedir(directory);
    snprintf(python_status, sizeof(python_status), "Python ready; loaded %d plugin%s", python_plugin_count, python_plugin_count == 1 ? "" : "s");
    return 1;
}

const char *vibs_python_status(void) {
    return python_status;
}

#pragma once

#include <Python.h> 
#include <exception>

bool start_python();

bool check_for_python_error();

bool check_return_value(PyObject *Obj, char *CallName);
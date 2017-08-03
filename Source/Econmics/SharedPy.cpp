
#include "Econmics.h"
#include "SharedPy.h"

// This is how we start Python
bool start_python()
{
	UE_LOG(LogTemp, Log, TEXT("[PYTHON] Starting python"));
	Py_NoSiteFlag = 1;

	FString PythonBinPath = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	PythonBinPath = FPaths::Combine(PythonBinPath, FString("Binaries"));
	PythonBinPath = FPaths::Combine(PythonBinPath, FString("Win64"));
	PythonBinPath = FPaths::Combine(PythonBinPath, FString("python36.zip"));
	UE_LOG(LogTemp, Log, TEXT("The PythonBinPath path is %s"), *PythonBinPath);

	FString PyProjPath = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	PyProjPath = FPaths::Combine(PyProjPath, FString("ThirdParty"));
	PyProjPath = FPaths::Combine(PyProjPath, FString("PythonHome"));
	PyProjPath = FPaths::Combine(PyProjPath, FString("PyEquilibria"));
	UE_LOG(LogTemp, Log, TEXT("The PyProjPath path is %s"), *PyProjPath);

	FString PyPathSetup = PythonBinPath + FString(TEXT(";")) + PyProjPath;

	// FString -> wchar_t* example. There is a variation of Py_DecodeLocale function for PyObject* creation.
	const TCHAR *pth = *PyPathSetup;  // const as must
	wchar_t *pypathbin = Py_DecodeLocale(TCHAR_TO_UTF8(pth), NULL);

	Py_SetPath(pypathbin);
	//Py_SetPath(L"E:\\Code\\Unreal Projects\\Econmics\\Binaries\\Win64\\python36.zip;E:\\Code\\Unreal Projects\\Econmics\\ThirdParty\\PythonHome\\PyEquilibria\\;");
	if (check_for_python_error()) { return false; };
	Py_Initialize();
	if (check_for_python_error()) { return false; };
	return true;

}


// Do this after each python call.
bool check_for_python_error()
{
	PyObject *pyerr = PyErr_Occurred();  // not borrowed
	if (PyErr_Occurred() != NULL)
	{

		// Get the error info (this resets the error state)
		PyObject *type, *value, *traceback;
		PyErr_Fetch(&type, &value, &traceback);

		PyErr_Clear();  // We know there is an error, but we need a few more commands to send the report

		PyObject *py_err_repr = PyObject_Repr(value);  // new reference (borrowed)
		PyObject *py_traceback = PyObject_Repr(traceback);  // May be this is a string
		if ((py_err_repr == NULL) || (py_traceback == NULL))
		{
			UE_LOG(LogTemp, Warning, TEXT("[PYTHON] unknown error"));  // this is an exception
																	  
			Py_Finalize();
			return true;
		};

		char *err_repr = PyUnicode_AsUTF8(py_err_repr);
		FString Fs_err = FString(UTF8_TO_TCHAR(err_repr));
		Py_DECREF(py_err_repr);

		char *err_traceback = PyUnicode_AsUTF8(py_traceback);
		FString Fs_traceback = FString(UTF8_TO_TCHAR(err_traceback));
		Py_DECREF(py_traceback);

		UE_LOG(LogTemp, Warning, TEXT("[PYTHON] error: \n %s \n %s"), *Fs_err, *Fs_traceback);
		
		// Restore the error state back (why not).
		PyErr_Restore(type, value, traceback);

		Py_Finalize();
		return true;

	};

	return false;
}


// Call this if python should return something other than NULL. Raises an exception.
bool check_return_value(PyObject *Obj, char *CallName = "")
{
	if (Obj == NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PYTHON] return of a call &s is NULL"), *FString(UTF8_TO_TCHAR(CallName)));
		if (check_for_python_error()) { return false; };
		UE_LOG(LogTemp, Warning, TEXT("[PYTHON] exception with return value without a python exception"));
		return false;
	};

	return true;
}



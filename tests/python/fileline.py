import sys
import traceback

def custom_excepthook(exc_type, exc_value, exc_tb):
    tb = traceback.extract_tb(exc_tb)
    for filename, lineno, func, text in tb:
        print(f"{filename}:{lineno}:1")
    print(f"{exc_type.__name__}: {exc_value}")

sys.excepthook = custom_excepthook

%module pymyframe

%{
#include "pyapp.h"
#include "pymsg.h"
%}

%include <std_shared_ptr.i>
%include <std_string.i>

%include "pymsg.h"
%include "pyapp.h"

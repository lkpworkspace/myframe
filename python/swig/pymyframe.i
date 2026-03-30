%module(directors="1") pymyframe

%feature("director") Actor;

%{
#include "pymsg.h"
#include "pyactor.h"
#include "pyapp.h"
%}

%include <std_shared_ptr.i>
%include <std_string.i>

%include "pymsg.h"
%include "pyactor.h"
%include "pyapp.h"

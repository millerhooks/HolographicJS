﻿#pragma once

#include <collection.h>
#include <ppltasks.h>

// Enable function definitions in the GL headers below
#define GL_GLEXT_PROTOTYPES

// OpenGL ES includes
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// EGL includes
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

// ANGLE include for Windows Store
#include <angle_windowsstore.h>

// Chakra engine with Edge mode enabled
#define USE_EDGEMODE_JSRT
#include <jsrt.h>
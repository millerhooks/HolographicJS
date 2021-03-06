#include "pch.h"
#include "Binding.h"
#include "CanvasRenderingContextHolographic.h"
#include <assert.h>
#include <time.h>

Engine* Binding::engine;

void Binding::bind() {
	JsValueRef globalObject;
	JsGetGlobalObject(&globalObject);

	// Example of how to bind a native inspectable
	//JsValueRef jsCoreWindow;
	//IInspectable* inspectableCoreWindow = reinterpret_cast<IInspectable*>(coreWindow);
	//JsInspectableToObject(inspectableCoreWindow, &jsCoreWindow);
	//setProperty(globalObject, L"coreWindow", jsCoreWindow);

	setCallback(globalObject, L"setTimeout", JSSetTimeout, nullptr);
	setCallback(globalObject, L"setInterval", JSSetInterval, nullptr);
	setCallback(globalObject, L"requestAnimationFrame", JSRequestAnimationFrame, nullptr);
}

// JsNativeFunction for setTimeout(func, delay)
JsValueRef CALLBACK Binding::JSSetTimeout(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 3);
	JsValueRef func = arguments[1];
	int delay = 0;
	JsNumberToInt(arguments[2], &delay);
	engine->taskQueue.push(new Task(func, delay, arguments[0], JS_INVALID_REFERENCE));
	return JS_INVALID_REFERENCE;
}

// JsNativeFunction for setInterval(func, delay)
JsValueRef CALLBACK Binding::JSSetInterval(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 3);
	JsValueRef func = arguments[1];
	int delay = 0;
	JsNumberToInt(arguments[2], &delay);
	engine->taskQueue.push(new Task(func, delay, arguments[0], JS_INVALID_REFERENCE, true));
	return JS_INVALID_REFERENCE;
}

// JsNativeFunction for requestAnimationFrame(func)
JsValueRef CALLBACK Binding::JSRequestAnimationFrame(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 2);
	CanvasRenderingContextHolographic::render();
	JsValueRef func = arguments[1];
	engine->taskQueue.push(new Task(func, 0, arguments[0], JS_INVALID_REFERENCE, true));
	return JS_INVALID_REFERENCE;
}

void Binding::setCallback(JsValueRef object, const wchar_t *propertyName, JsNativeFunction callback, void *callbackState)
{
	JsPropertyIdRef propertyId;
	JsGetPropertyIdFromName(propertyName, &propertyId);
	JsValueRef function;
	JsCreateFunction(callback, callbackState, &function);
	JsSetProperty(object, propertyId, function, true);
}

void Binding::setProperty(JsValueRef object, const wchar_t *propertyName, JsValueRef property)
{
	JsPropertyIdRef propertyId;
	JsGetPropertyIdFromName(propertyName, &propertyId);
	JsSetProperty(object, propertyId, property, true);
}

JsValueRef Binding::getProperty(JsValueRef object, const wchar_t *propertyName)
{
	JsValueRef output;
	JsPropertyIdRef propertyId;
	JsGetPropertyIdFromName(propertyName, &propertyId);
	JsGetProperty(object, propertyId, &output);
	return output;
}

void Binding::projectNativeClassToGlobal(const wchar_t *className, JsNativeFunction constructor, JsValueRef &prototype, map<const wchar_t *, JsNativeFunction> members, map<const wchar_t *, JsValueRef> properties) {
	JsValueRef globalObject;
	JsGetGlobalObject(&globalObject);
	return projectNativeClassToObject(globalObject, className, constructor, prototype, members, properties);
}

void Binding::projectNativeClassToObject(JsValueRef object, const wchar_t *className, JsNativeFunction constructor, JsValueRef &prototype, map<const wchar_t *, JsNativeFunction> members, map<const wchar_t *, JsValueRef> properties) {
	JsValueRef jsConstructor;
	JsCreateFunction(constructor, nullptr, &jsConstructor);
	setProperty(object, className, jsConstructor);

	JsCreateObject(&prototype);

	map<const wchar_t *, JsNativeFunction>::iterator memberIterator = members.begin();
	while (memberIterator != members.end())
	{
		setCallback(prototype, memberIterator->first, memberIterator->second, nullptr);
		memberIterator++;
	}

	map<const wchar_t *, JsValueRef>::iterator propertyIterator = properties.begin();
	while (propertyIterator != properties.end())
	{
		setProperty(prototype, propertyIterator->first, propertyIterator->second);
		propertyIterator++;
	}

	setProperty(jsConstructor, L"prototype", prototype);
}

HolographicSpace^ Binding::valueToHolographicSpace(JsValueRef value) {
	// Use once HolographicSpace native pointer is available
	//void* hs;
	//JsGetExternalData(value, &hs);
	//return reinterpret_cast<HolographicSpace^>(hs);
	IInspectable* holographicSpace;
	JsObjectToInspectable(value, &holographicSpace);
	return reinterpret_cast<HolographicSpace^>(holographicSpace);
}

SpatialStationaryFrameOfReference^ Binding::valueToSpatialStationaryFrameOfReference(JsValueRef value) {
	// Use once HolographicSpace native pointer is available
	//void* srf;
	//JsGetExternalData(value, &srf);
	//return reinterpret_cast<SpatialStationaryFrameOfReference^>(srf);
	IInspectable* srf;
	JsObjectToInspectable(value, &srf);
	return reinterpret_cast<SpatialStationaryFrameOfReference^>(srf);
}

WebGLActiveInfo* Binding::valueToWebGLActiveInfo(JsValueRef value) {
	void* activeInfo;
	JsGetExternalData(value, &activeInfo);
	return reinterpret_cast<WebGLActiveInfo*>(activeInfo);
}

bool Binding::valueToBool(JsValueRef value) {
	bool b;
	JsBooleanToBool(value, &b);
	return b;
}

double Binding::valueToDouble(JsValueRef value) {
	double d;
	JsNumberToDouble(value, &d);
	return d;
}

float Binding::valueToFloat(JsValueRef value) {
	double d;
	JsNumberToDouble(value, &d);
	return (float)d;
}

int Binding::valueToInt(JsValueRef value) {
	int i;
	JsNumberToInt(value, &i);
	return i;
}

std::vector<float> Binding::valueToFloatVector(JsValueRef value) {
	int length;
	std::vector<float> floats;
	JsNumberToInt(getProperty(value, L"length"), &length);
	for (int i = 0; i < length; i++) {
		JsValueRef jsIndex;
		JsIntToNumber(i, &jsIndex);
		JsValueRef jsNumber;
		JsGetIndexedProperty(value, jsIndex, &jsNumber);
		floats.push_back(valueToFloat(jsNumber));
	}

	return floats;
}

std::vector<int> Binding::valueToIntVector(JsValueRef value) {
	int length;
	std::vector<int> ints;
	JsNumberToInt(getProperty(value, L"length"), &length);
	for (int i = 0; i < length; i++) {
		JsValueRef jsIndex;
		JsIntToNumber(i, &jsIndex);
		JsValueRef jsNumber;
		JsGetIndexedProperty(value, jsIndex, &jsNumber);
		ints.push_back(valueToInt(jsNumber));
	}

	return ints;
}

int Binding::valueToShort(JsValueRef value) {
	int i;
	JsNumberToInt(value, &i);
	return (short)i;
}

std::vector<short> Binding::valueToShortVector(JsValueRef value) {
	int length;
	std::vector<short> shorts;
	JsNumberToInt(getProperty(value, L"length"), &length);
	for (int i = 0; i < length; i++) {
		JsValueRef jsIndex;
		JsIntToNumber(i, &jsIndex);
		JsValueRef jsNumber;
		JsGetIndexedProperty(value, jsIndex, &jsNumber);
		shorts.push_back(valueToShort(jsNumber));
	}

	return shorts;
}

const wchar_t * Binding::valueToString(JsValueRef value) {
	const wchar_t * str;
	size_t length;
	JsStringToPointer(value, &str, &length);

	return str;
}

JsValueRef Binding::boolToValue(bool val) {
	JsValueRef b;
	JsBoolToBoolean(val, &b);
	return b;
}

JsValueRef Binding::boolVectorToValue(std::vector<bool> vec) {
	JsValueRef arr;
	JsCreateArray(vec.size(), &arr);

	int i = 0;
	for (std::vector<bool>::iterator it = vec.begin(); it != vec.end(); ++it) {

		JsValueRef value;
		JsBoolToBoolean(*it, &value);

		JsValueRef index;
		JsIntToNumber(i, &index);

		JsSetIndexedProperty(arr, index, value);

		i++;
	}

	return arr;
}

JsValueRef Binding::doubleVectorToValue(std::vector<double> vec) {
	JsValueRef arr;
	JsCreateArray(vec.size(), &arr);

	int i = 0;
	for (std::vector<double>::iterator it = vec.begin(); it != vec.end(); ++it) {

		JsValueRef value;
		JsDoubleToNumber(*it, &value);

		JsValueRef index;
		JsIntToNumber(i, &index);

		JsSetIndexedProperty(arr, index, value);

		i++;
	}

	return arr;
}

JsValueRef Binding::floatVectorToValue(std::vector<float> vec) {
	JsValueRef arr;
	JsCreateArray(vec.size(), &arr);

	int i = 0;
	for (std::vector<float>::iterator it = vec.begin(); it != vec.end(); ++it) {

		JsValueRef value;
		JsDoubleToNumber(*it, &value);

		JsValueRef index;
		JsIntToNumber(i, &index);

		JsSetIndexedProperty(arr, index, value);

		i++;
	}

	return arr;
}

JsValueRef Binding::intToValue(int val) {
	JsValueRef number;
	JsIntToNumber(val, &number);

	return number;
}

JsValueRef Binding::intVectorToValue(std::vector<int> vec) {
	JsValueRef arr;
	JsCreateArray(vec.size(), &arr);

	int i = 0;
	for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); ++it) {

		JsValueRef value;
		JsIntToNumber(*it, &value);

		JsValueRef index;
		JsIntToNumber(i, &index);

		JsSetIndexedProperty(arr, index, value);

		i++;
	}

	return arr;
}

JsValueRef Binding::stringToValue(const wchar_t * str, size_t strLen) {
	JsValueRef value;
	JsPointerToString(str, strLen, &value);
	return value;
}

JsValueRef Binding::stringVectorToValue(std::vector<const wchar_t *> vec) {
	JsValueRef arr;
	JsCreateArray(vec.size(), &arr);

	int i = 0;
	for (std::vector<const wchar_t *>::iterator it = vec.begin(); it != vec.end(); ++it) {

		JsValueRef value;
		JsPointerToString(*it, wcslen(*it), &value);

		JsValueRef index;
		JsIntToNumber(i, &index);

		JsSetIndexedProperty(arr, index, value);

		i++;
	}

	return arr;
}

JsValueRef Binding::webGLActiveInfoToValue(WebGLActiveInfo activeInfo) {
	JsValueRef value;
	JsCreateExternalObject(&activeInfo, nullptr, &value);

	setProperty(value, L"type", intToValue(activeInfo.type));
	setProperty(value, L"name", stringToValue(activeInfo.name, wcslen(activeInfo.name)));
	setProperty(value, L"size", intToValue(activeInfo.size));

	return value;
}

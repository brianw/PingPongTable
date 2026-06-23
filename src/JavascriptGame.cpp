#include "JavascriptGame.h"

#include <algorithm>

namespace {
    JavascriptGame* game(JSContext* context) {
        return static_cast<JavascriptGame*>(JS_GetContextOpaque(context));
    }

    std::string valueToString(JSContext* context, JSValueConst value) {
        const char* text = JS_ToCString(context, value);
        if (!text) {
            return "";
        }
        std::string result(text);
        JS_FreeCString(context, text);
        return result;
    }

    [[noreturn]] void throwException(JSContext* context) {
        JSValue exception = JS_GetException(context);
        std::string message = valueToString(context, exception);
        JS_FreeValue(context, exception);
        throw std::runtime_error(message);
    }

    void checkException(JSContext* context, JSValue value) {
        if (JS_IsException(value)) {
            throwException(context);
        }
    }

    bool numberArgument(JSContext* context, JSValueConst* arguments, int argumentCount, int index, double& value) {
        if (index >= argumentCount) {
            JS_ThrowTypeError(context, "expected numeric argument");
            return false;
        }
        if (JS_ToFloat64(context, &value, arguments[index]) < 0) {
            return false;
        }
        return true;
    }

    ofColor namedColor(const std::string& name, ofColor defaultColor) {
        if (name == "black") {
            return ofColor::black;
        }
        if (name == "white") {
            return ofColor::white;
        }
        if (name == "red") {
            return ofColor::red;
        }
        if (name == "green") {
            return ofColor::green;
        }
        if (name == "blue") {
            return ofColor::blue;
        }
        if (name == "yellow") {
            return ofColor::yellow;
        }
        if (name == "cyan") {
            return ofColor::cyan;
        }
        if (name == "magenta") {
            return ofColor::magenta;
        }
        return defaultColor;
    }

    int hexByte(const std::string& color, int index) {
        return std::stoi(color.substr(index, 2), nullptr, 16);
    }

    ofColor colorFromValue(JSContext* context, JSValueConst value, ofColor defaultColor) {
        if (JS_IsString(value)) {
            std::string color = valueToString(context, value);
            if (color.size() == 7 && color[0] == '#') {
                return ofColor(hexByte(color, 1), hexByte(color, 3), hexByte(color, 5));
            }
            return namedColor(color, defaultColor);
        }

        if (JS_IsArray(context, value)) {
            JSValue lengthValue = JS_GetPropertyStr(context, value, "length");
            checkException(context, lengthValue);
            int32_t length = 0;
            JS_ToInt32(context, &length, lengthValue);
            JS_FreeValue(context, lengthValue);
            if (length >= 3) {
                int values[4] = {0, 0, 0, 255};
                for (int i = 0; i < std::min(length, 4); i++) {
                    JSValue item = JS_GetPropertyUint32(context, value, i);
                    checkException(context, item);
                    JS_ToInt32(context, &values[i], item);
                    JS_FreeValue(context, item);
                }
                return ofColor(values[0], values[1], values[2], values[3]);
            }
        }

        return defaultColor;
    }

    ofColor colorFromArguments(JSContext* context, JSValueConst* arguments, int argumentCount, ofColor defaultColor) {
        if (argumentCount == 0) {
            return defaultColor;
        }
        if (JS_IsNumber(arguments[0])) {
            int values[4] = {255, 255, 255, 255};
            for (int i = 0; i < std::min(argumentCount, 4); i++) {
                JS_ToInt32(context, &values[i], arguments[i]);
            }
            if (argumentCount == 1) {
                values[1] = values[0];
                values[2] = values[0];
            }
            return ofColor(values[0], values[1], values[2], values[3]);
        }
        return colorFromValue(context, arguments[0], defaultColor);
    }

    JSValue gClear(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        ofClear(colorFromArguments(context, arguments, argumentCount, ofColor::black));
        return JS_UNDEFINED;
    }

    JSValue gFill(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        game(context)->setFillColor(colorFromArguments(context, arguments, argumentCount, ofColor::white));
        return JS_UNDEFINED;
    }

    JSValue gNoFill(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        game(context)->disableFill();
        return JS_UNDEFINED;
    }

    JSValue gStroke(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        game(context)->setStrokeColor(colorFromArguments(context, arguments, argumentCount, ofColor::white));
        return JS_UNDEFINED;
    }

    JSValue gNoStroke(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        game(context)->disableStroke();
        return JS_UNDEFINED;
    }

    JSValue gStrokeWidth(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        double width = 1;
        if (!numberArgument(context, arguments, argumentCount, 0, width)) {
            return JS_EXCEPTION;
        }
        game(context)->setStrokeWidth(width);
        return JS_UNDEFINED;
    }

    JSValue gCircle(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        double x = 0;
        double y = 0;
        double radius = 0;
        if (!numberArgument(context, arguments, argumentCount, 0, x) ||
            !numberArgument(context, arguments, argumentCount, 1, y) ||
            !numberArgument(context, arguments, argumentCount, 2, radius)) {
            return JS_EXCEPTION;
        }
        game(context)->drawCircle(x, y, radius);
        return JS_UNDEFINED;
    }

    JSValue gRect(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        double x = 0;
        double y = 0;
        double width = 0;
        double height = 0;
        if (!numberArgument(context, arguments, argumentCount, 0, x) ||
            !numberArgument(context, arguments, argumentCount, 1, y) ||
            !numberArgument(context, arguments, argumentCount, 2, width) ||
            !numberArgument(context, arguments, argumentCount, 3, height)) {
            return JS_EXCEPTION;
        }
        game(context)->drawRect(x, y, width, height);
        return JS_UNDEFINED;
    }

    JSValue gLine(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        double x1 = 0;
        double y1 = 0;
        double x2 = 0;
        double y2 = 0;
        if (!numberArgument(context, arguments, argumentCount, 0, x1) ||
            !numberArgument(context, arguments, argumentCount, 1, y1) ||
            !numberArgument(context, arguments, argumentCount, 2, x2) ||
            !numberArgument(context, arguments, argumentCount, 3, y2)) {
            return JS_EXCEPTION;
        }
        game(context)->drawLine(x1, y1, x2, y2);
        return JS_UNDEFINED;
    }

    JSValue gPoint(JSContext* context, JSValueConst thisValue, int argumentCount, JSValueConst* arguments) {
        double x = 0;
        double y = 0;
        if (!numberArgument(context, arguments, argumentCount, 0, x) ||
            !numberArgument(context, arguments, argumentCount, 1, y)) {
            return JS_EXCEPTION;
        }
        game(context)->drawPoint(x, y);
        return JS_UNDEFINED;
    }

    void setFunction(JSContext* context, JSValueConst object, const char* name, JSCFunction* callback, int argumentCount) {
        JS_SetPropertyStr(context, object, name, JS_NewCFunction(context, callback, name, argumentCount));
    }

    JSValue makeTableObject(JSContext* context, ofVec2f canvasSize) {
        JSValue table = JS_NewObject(context);
        JS_SetPropertyStr(context, table, "width", JS_NewFloat64(context, canvasSize.x));
        JS_SetPropertyStr(context, table, "height", JS_NewFloat64(context, canvasSize.y));
        JS_SetPropertyStr(context, table, "frame", JS_NewInt32(context, ofGetFrameNum()));
        JS_SetPropertyStr(context, table, "time", JS_NewFloat64(context, ofGetElapsedTimef()));
        return table;
    }

    JSValue makeHitObject(JSContext* context, ofVec2f location, bool isNearSide) {
        JSValue hit = JS_NewObject(context);
        JS_SetPropertyStr(context, hit, "x", JS_NewFloat64(context, location.x));
        JS_SetPropertyStr(context, hit, "y", JS_NewFloat64(context, location.y));
        JS_SetPropertyStr(context, hit, "side", JS_NewString(context, isNearSide ? "near" : "far"));
        return hit;
    }

    void requireFunction(JSContext* context, JSValueConst global, const char* name) {
        JSValue value = JS_GetPropertyStr(context, global, name);
        checkException(context, value);
        bool isFunction = JS_IsFunction(context, value);
        JS_FreeValue(context, value);
        if (!isFunction) {
            throw std::runtime_error(std::string("JavascriptGame requires function ") + name);
        }
    }

    void callFunction(JSContext* context, const char* name, std::vector<JSValue> arguments) {
        JSValue global = JS_GetGlobalObject(context);
        JSValue function = JS_GetPropertyStr(context, global, name);
        checkException(context, function);
        JSValue result = JS_Call(context, function, global, arguments.size(), arguments.data());
        JS_FreeValue(context, function);
        JS_FreeValue(context, global);
        checkException(context, result);
        JS_FreeValue(context, result);
    }
}

JavascriptGame::JavascriptGame(std::string codePath)
    : codePath(codePath)
    , gameName(ofFilePath::getBaseName(codePath))
    , fillEnabled(true)
    , strokeEnabled(false)
    , currentStrokeWidth(1)
    , fillColor(ofColor::white)
    , strokeColor(ofColor::white)
    , runtime(nullptr)
    , context(nullptr)
    , graphicsObject(JS_UNDEFINED) {
}

JavascriptGame::~JavascriptGame() {
    if (context) {
        JS_FreeValue(context, graphicsObject);
        JS_FreeContext(context);
    }
    if (runtime) {
        JS_FreeRuntime(runtime);
    }
}

std::string JavascriptGame::name() const {
    return gameName;
}

void JavascriptGame::setup(ofVec2f size) {
    canvasSize = size;

    runtime = JS_NewRuntime();
    if (!runtime) {
        throw std::runtime_error("JavascriptGame could not create QuickJS runtime");
    }
    context = JS_NewContext(runtime);
    if (!context) {
        throw std::runtime_error("JavascriptGame could not create QuickJS context");
    }
    JS_SetContextOpaque(context, this);

    graphicsObject = JS_NewObject(context);
    setFunction(context, graphicsObject, "clear", gClear, 1);
    setFunction(context, graphicsObject, "fill", gFill, 1);
    setFunction(context, graphicsObject, "noFill", gNoFill, 0);
    setFunction(context, graphicsObject, "stroke", gStroke, 1);
    setFunction(context, graphicsObject, "noStroke", gNoStroke, 0);
    setFunction(context, graphicsObject, "strokeWidth", gStrokeWidth, 1);
    setFunction(context, graphicsObject, "circle", gCircle, 3);
    setFunction(context, graphicsObject, "rect", gRect, 4);
    setFunction(context, graphicsObject, "line", gLine, 4);
    setFunction(context, graphicsObject, "point", gPoint, 2);

    ofFile scriptFile(ofToDataPath(codePath));
    if (!scriptFile.exists()) {
        throw std::runtime_error("JavascriptGame could not find " + codePath);
    }

    ofBuffer script = ofBufferFromFile(scriptFile.getAbsolutePath());
    JSValue result = JS_Eval(context, script.getText().c_str(), script.size(), scriptFile.getAbsolutePath().c_str(), JS_EVAL_TYPE_GLOBAL);
    checkException(context, result);
    JS_FreeValue(context, result);

    JSValue global = JS_GetGlobalObject(context);
    requireFunction(context, global, "setup");
    requireFunction(context, global, "strike");
    requireFunction(context, global, "update");
    requireFunction(context, global, "draw");
    JS_FreeValue(context, global);

    JSValue table = makeTableObject(context, canvasSize);
    callFunction(context, "setup", {table});
    JS_FreeValue(context, table);
}

void JavascriptGame::update() {
    JSValue dt = JS_NewFloat64(context, ofGetLastFrameTime());
    JSValue table = makeTableObject(context, canvasSize);
    callFunction(context, "update", {dt, table});
    JS_FreeValue(context, dt);
    JS_FreeValue(context, table);
}

void JavascriptGame::draw(ofFbo& output, int brightness) {
    JSValue table = makeTableObject(context, canvasSize);

    output.begin();
    ofClear(0, 0, 0);
    ofSetColor(brightness);
    ofPushStyle();
    callFunction(context, "draw", {graphicsObject, table});
    ofPopStyle();
    output.end();

    JS_FreeValue(context, table);
}

void JavascriptGame::handleStrike(ofVec2f location, bool isNearSide) {
    JSValue hit = makeHitObject(context, location, isNearSide);
    JSValue table = makeTableObject(context, canvasSize);
    callFunction(context, "strike", {hit, table});
    JS_FreeValue(context, hit);
    JS_FreeValue(context, table);
}

void JavascriptGame::setFillColor(ofColor color) {
    fillColor = color;
    fillEnabled = true;
}

void JavascriptGame::disableFill() {
    fillEnabled = false;
}

void JavascriptGame::setStrokeColor(ofColor color) {
    strokeColor = color;
    strokeEnabled = true;
}

void JavascriptGame::disableStroke() {
    strokeEnabled = false;
}

void JavascriptGame::setStrokeWidth(float width) {
    currentStrokeWidth = width;
}

void JavascriptGame::drawCircle(float x, float y, float radius) {
    if (fillEnabled) {
        ofFill();
        ofSetColor(fillColor);
        ofDrawCircle(x, y, radius);
    }
    if (strokeEnabled) {
        ofNoFill();
        ofSetLineWidth(currentStrokeWidth);
        ofSetColor(strokeColor);
        ofDrawCircle(x, y, radius);
        ofSetLineWidth(1);
    }
}

void JavascriptGame::drawRect(float x, float y, float width, float height) {
    if (fillEnabled) {
        ofFill();
        ofSetColor(fillColor);
        ofDrawRectangle(x, y, width, height);
    }
    if (strokeEnabled) {
        ofNoFill();
        ofSetLineWidth(currentStrokeWidth);
        ofSetColor(strokeColor);
        ofDrawRectangle(x, y, width, height);
        ofSetLineWidth(1);
    }
}

void JavascriptGame::drawLine(float x1, float y1, float x2, float y2) {
    if (!strokeEnabled) {
        return;
    }
    ofSetLineWidth(currentStrokeWidth);
    ofSetColor(strokeColor);
    ofDrawLine(x1, y1, x2, y2);
    ofSetLineWidth(1);
}

void JavascriptGame::drawPoint(float x, float y) {
    ofSetColor(strokeEnabled ? strokeColor : fillColor);
    ofDrawRectangle(x, y, 1, 1);
}

/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if defined(TARGET_IPHONE_SIMULATOR) or defined(TARGET_OS_IPHONE) // iOS

#include "minko/Common.hpp"
#include "ioswebview/dom/IOSWebViewDOM.hpp"
#include "ioswebview/dom/IOSWebViewDOMEngine.hpp"

using namespace minko;
using namespace minko::dom;
using namespace ioswebview;
using namespace ioswebview::dom;

IOSWebViewDOM::IOSWebViewDOM(std::string jsAccessor) :
	_initialized(false),
	_onload(Signal<AbstractDOM::Ptr, std::string>::create()),
	_onmessage(Signal<AbstractDOM::Ptr, std::string>::create()),
	_jsAccessor(jsAccessor),
    _engine(nullptr)
{
}

IOSWebViewDOM::Ptr
IOSWebViewDOM::create(std::string jsAccessor, std::shared_ptr<IOSWebViewDOMEngine> engine)
{
	Ptr dom(new IOSWebViewDOM(jsAccessor));
    dom->_engine = engine;
    
	return dom;
}

void
IOSWebViewDOM::sendMessage(std::string message, bool async)
{
	//std::string eval = "if (" + _jsAccessor + ".window.Minko.onmessage) " + _jsAccessor + ".window.Minko.onmessage('" + message + "');";
	//if (!async)
		//emscripten_run_script(eval.c_str());
	//else
	//	emscripten_async_run_script("console.log('toto'); if (" + _jsAccessor + ".window.Minko.onmessage) " + _jsAccessor + ".window.Minko.onmessage('" + message + "');", 1);

    WebViewJavascriptBridge* bridge = _engine->bridge();
    [bridge send:[NSString stringWithCString:message.c_str() encoding:[NSString defaultCStringEncoding]]];
}

void
IOSWebViewDOM::eval(std::string message, bool async)
{
	//if (!async)
		std::string ev = _jsAccessor + ".window.eval('" + message + "')";
		//emscripten_run_script(ev.c_str());
	//else
	//	emscripten_async_run_script(message.c_str(), 1);
}

std::vector<AbstractDOMElement::Ptr>
IOSWebViewDOM::getElementList(std::string expression)
{
	std::vector<minko::dom::AbstractDOMElement::Ptr> l;

	expression = "Minko.tmpElements = " + expression;

	//emscripten_run_script(expression.c_str());
    /*
	expression = "(Minko.tmpElements.length)";
	int numElements = emscripten_run_script_int(expression.c_str());

	for(int i = 0; i < numElements; ++i)
		l.push_back(IOSWebViewDOMElement::getDOMElement("Minko.tmpElements[" + std::to_string(i) + "]"));
    */
	return l;
}

AbstractDOMElement::Ptr
IOSWebViewDOM::createElement(std::string element)
{
    /*
	std::string eval = "Minko.tmpElement = " + _jsAccessor + ".document.createElement('" + element + "');";

	//emscripten_run_script(eval.c_str());

	return IOSWebViewDOMElement::getDOMElement("Minko.tmpElement");
     */
    
    return nullptr;
}

AbstractDOMElement::Ptr
IOSWebViewDOM::getElementById(std::string id)
{
	std::string eval = "Minko.tmpElement = " + _jsAccessor + ".document.getElementById('" + id + "');";

	//emscripten_run_script(eval.c_str());

	//return IOSWebViewDOMElement::getDOMElement("Minko.tmpElement");
    
    return nullptr;
}

std::vector<AbstractDOMElement::Ptr>
IOSWebViewDOM::getElementsByClassName(std::string className)
{
	return getElementList(_jsAccessor + ".document.getElementsByClassName('" + className + "')");
}

std::vector<AbstractDOMElement::Ptr>
IOSWebViewDOM::getElementsByTagName(std::string tagName)
{
	return getElementList(_jsAccessor + ".document.getElementsByTagName('" + tagName + "')");
}

AbstractDOMElement::Ptr
IOSWebViewDOM::document()
{
	return _document;
}

AbstractDOMElement::Ptr
IOSWebViewDOM::body()
{
	return _body;
}

Signal<AbstractDOM::Ptr, std::string>::Ptr
IOSWebViewDOM::onload()
{
	return _onload;
}

Signal<AbstractDOM::Ptr, std::string>::Ptr
IOSWebViewDOM::onmessage()
{
	return _onmessage;
}

std::string
IOSWebViewDOM::fileName()
{
	std::string completeUrl = fullUrl();
	int i = completeUrl.find_last_of('/');

	return completeUrl.substr(i + 1);
}

std::string
IOSWebViewDOM::fullUrl()
{
	std::string eval = "(" + _jsAccessor + ".document.location)";

	//char* result = emscripten_run_script_string(eval.c_str());

	return std::string(/*result*/);
}

bool
IOSWebViewDOM::isMain()
{
	return true;
}

void
IOSWebViewDOM::initialized(bool v)
{
	if (!_initialized && v)
	{
		std::string eval = "";
		eval += _jsAccessor + ".window		= Minko.iframeElement.contentWindow;\n";
		eval += _jsAccessor + ".document	= Minko.iframeElement.contentDocument;\n";
		eval += _jsAccessor + ".body		= Minko.iframeElement.contentDocument.body;\n";
		
        //emscripten_run_script(eval.c_str());

        /*
		_document	= IOSWebViewDOMElement::create(_jsAccessor + ".document");
		_body		= IOSWebViewDOMElement::create(_jsAccessor + ".body");
        */
	}

	_initialized = v;
}

bool
IOSWebViewDOM::initialized()
{
	return _initialized;
}
#endif
#include "QMinkoEffectEditor.hpp"

#include "ui/ui_QMinkoEffectEditor.h"
#include <QtWidgets/QToolButton>
#include <QtWidgets/QFileDialog>
#include <QtWebKitWidgets/QWebFrame>

#include <minko/file/Loader.hpp>
#include <minko/file/EffectParser.hpp>

using namespace minko;

QMinkoEffectEditor::QMinkoEffectEditor(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::QMinkoEffectEditor),
	_qVertexWebFrame(nullptr),
	_qVertexObjectJS(nullptr),
	_qVertexShaderSource(),
	_qFragmentWebFrame(nullptr),
	_qFragmentObjectJS(nullptr),
	_qFragmentShaderSource(),
	_qBindingsSource(),
	_saveNeeded(false),
	_effectParserCompleteSlot(nullptr)
{
    _ui->setupUi(this);

	QObject::connect(_ui->effectNameLineEdit, SIGNAL(editingFinished()), this, SLOT(updateEffectName()));

	setupIOButtons();
	setupSourceTabs();
}

QMinkoEffectEditor::~QMinkoEffectEditor()
{
    delete _ui;
	
	if (_qIconSave)
		delete _qIconSave;
	if (_qIconSaveNeeded)
		delete _qIconSaveNeeded;

	delete _qVertexObjectJS;
	delete _qFragmentObjectJS;

	_effectParserCompleteSlot = nullptr;
}

void
QMinkoEffectEditor::setupIOButtons()
{
	_qIconSave			= new QIcon(":/resources/icon-save-effect.png");
	_qIconSaveNeeded	= new QIcon(":/resources/icon-save-effect-needed.png");

	_ui->loadMkToolButton		->setIcon(QIcon(":/resources/icon-load-mk.png"));
	_ui->loadEffectToolButton	->setIcon(QIcon(":/resources/icon-load-effect.png"));

	saveNeeded(false);

	QObject::connect(_ui->loadMkToolButton,		SIGNAL(released()),	this,	SLOT(loadMk()));
	QObject::connect(_ui->loadEffectToolButton,	SIGNAL(released()),	this,	SLOT(loadEffect()));
	QObject::connect(_ui->saveEffectToolButton,	SIGNAL(released()),	this,	SLOT(saveEffect()));
}

void
QMinkoEffectEditor::setupSourceTabs()
{
	_qVertexWebFrame	= _ui->vertexWebView->page()->mainFrame();
	_qVertexWebFrame	->load(QUrl("qrc:///resources/minimal-codemirror.html"));
	_qVertexObjectJS	= new QObject();
	_qVertexObjectJS	->setProperty("type", (QString)"vertexShader");

	_qFragmentWebFrame	= _ui->fragmentWebView->page()->mainFrame();
	_qFragmentWebFrame	->load(QUrl("qrc:///resources/minimal-codemirror.html"));
	_qFragmentObjectJS	= new QObject();
	_qFragmentObjectJS	->setProperty("type", (QString)"fragmentShader");

	QObject::connect(_qVertexWebFrame,		SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(exposeQObjectsToVertexJS()));
	QObject::connect(_qFragmentWebFrame,	SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(exposeQObjectsToFragmentJS()));
}

void
QMinkoEffectEditor::loadMk(const QString& filename)
{
	std::cout << "load mk file: " << qPrintable(filename) << std::endl;
}

void
QMinkoEffectEditor::loadEffect(const QString& filename)
{
	std::cout << "load effect file: " << qPrintable(filename) << std::endl;

	auto sceneManager			= _ui->minkoWidget->sceneManager();
	auto loader					= file::Loader::create();
	auto effectParser			= file::EffectParser::create();
	_effectParserCompleteSlot	= effectParser->complete()->connect(std::bind(
		&QMinkoEffectEditor::effectParserCompleteHandler,
		this,
		std::placeholders::_1
	));

	loader->load(
		filename.toUtf8().constData(), 
		sceneManager->assets()->defaultOptions()
	);

	effectParser->parse(
		loader->filename(),
		loader->resolvedFilename(),
		sceneManager->assets()->defaultOptions(),
		loader->data(),
		sceneManager->assets()
	);
}

void
QMinkoEffectEditor::effectParserCompleteHandler(file::AbstractParser::Ptr iParser)
{
	file::EffectParser::Ptr parser = std::dynamic_pointer_cast<file::EffectParser>(iParser);
	if (parser == nullptr)
		return;

	// update ui to account for the newly-parsed effect file
	_ui->effectNameLineEdit->setText(parser->effectName().c_str());

	auto effect = parser->effect();
	auto passes = effect->passes();

	if (passes.size() > 1)
		std::cerr << "WARNING: only one pass shaders are handled for the time being." << std::endl;

	for (unsigned int i = 0; i < passes.size(); ++i)
	{
		auto pass = passes[i];
		
		std::string			vertexSource;
		std::string			fragmentSource;

		fix(pass->program()->vertexShader()->source(),		vertexSource);
		fix(pass->program()->fragmentShader()->source(),	fragmentSource);

		const std::string	vertexShaderJSCode		= "codeMirror.setValue(\"" + vertexSource + "\");";
		const std::string	fragmentShaderJSCode	= "codeMirror.setValue(\"" + fragmentSource + "\");";

		_qVertexWebFrame	->evaluateJavaScript(vertexShaderJSCode.c_str());
		_qFragmentWebFrame	->evaluateJavaScript(fragmentShaderJSCode.c_str());

		break;
	}
}



void
QMinkoEffectEditor::saveEffect(const QString& filename)
{
	std::cout << "save effect file: " << qPrintable(filename) << std::endl;
	saveNeeded(false);
}

void
QMinkoEffectEditor::exposeQObjectsToVertexJS()
{
	_ui->vertexWebView->page()->mainFrame()->addToJavaScriptWindowObject("qMinkoEffectEditor", this);
	_ui->vertexWebView->page()->mainFrame()->addToJavaScriptWindowObject("qObjectID", _qVertexObjectJS);
}

void
QMinkoEffectEditor::exposeQObjectsToFragmentJS()
{
	_ui->fragmentWebView->page()->mainFrame()->addToJavaScriptWindowObject("qMinkoEffectEditor", this);
	_ui->fragmentWebView->page()->mainFrame()->addToJavaScriptWindowObject("qObjectID", _qFragmentObjectJS);
}

void
QMinkoEffectEditor::saveNeeded(bool value)
{
	_saveNeeded	= value;
	_ui->saveEffectToolButton->setIcon(_saveNeeded ? *_qIconSaveNeeded : *_qIconSave);
}

void
QMinkoEffectEditor::createEffect(std::string& effect) const
{
	effect.clear();

	const std::string& effectName	(_ui->effectNameLineEdit->text()	.toUtf8().constData());
	const std::string& srcVertex	(_qVertexShaderSource				.toUtf8().constData());
	const std::string& srcFragment	(_qFragmentShaderSource				.toUtf8().constData());
	const std::string& srcBindings	(_qBindingsSource					.toUtf8().constData());

	effect = "effect name = " + effectName + "\n"
		+ "bindings = \n" + srcBindings + "\n"
		+ "vertex shader = \n" + srcVertex + "\n"
		+ "fragment shader = \n" + srcFragment + "\n";
}

void
QMinkoEffectEditor::displayEffect() const
{
	std::string effect;
	createEffect(effect);

	std::cout << "EFFECT\n" << effect << std::endl;
}

/*slot*/
void
QMinkoEffectEditor::updateEffectName()
{
	saveNeeded(true);
	displayEffect();
}

/*slot*/
void
QMinkoEffectEditor::updateSource(const QString& type)
{
	if (type == QString("vertexShader"))
		_qVertexShaderSource	= _qVertexWebFrame->evaluateJavaScript("codeMirror.getValue();").toString();
	else if (type == QString("fragmentShader"))
		_qFragmentShaderSource	= _qFragmentWebFrame->evaluateJavaScript("codeMirror.getValue();").toString();
	else
		throw std::invalid_argument("type");

	saveNeeded(true);
	displayEffect();
}

/*slot*/
void
QMinkoEffectEditor::loadMk()
{
	const QString& filename = QFileDialog::getOpenFileName(this, tr("Load *.mk file"), QString(), "MK files (*.mk)");
	if (filename.isEmpty())
		return;

	loadMk(filename);
}

/*slot*/
void
QMinkoEffectEditor::loadEffect()
{
	const QString& filename = QFileDialog::getOpenFileName(this, tr("Load *.effect file"), QString(), "Effect files (*.effect)");
	if (filename.isEmpty())
		return;

	loadEffect(filename);
}

/*slot*/
void
QMinkoEffectEditor::saveEffect()
{
	const QString& filename = QFileDialog::getSaveFileName(this, tr("Save *.effect file"), QString(), "Effect files (*.effect)");
	if (filename.isEmpty())
		return;

	saveEffect(filename);
}

/*static*/
void QMinkoEffectEditor::fix(const std::string& str,
							 std::string& res)
{
	std::string temp;
	removeLeftmostExtraTabs(str, temp);
	escapeSpecialCharacters(temp, res);
}

/*static*/
void QMinkoEffectEditor::removeLeftmostExtraTabs(const std::string& str, 
												 std::string& res)
{
	unsigned int numTabs = countLeftmostExtraTabs(str);

	res.clear();
	res.reserve(str.size());
	if (numTabs > 0)
	{
		unsigned int i = 0;
		while (i < str.size())
		{
			bool seqFound = true;

			if (i > 0)
				seqFound	= (str[i] == L'\n');
			unsigned int offset = i == 0 ? 0 : 1;
			for (unsigned int t = 0; t < numTabs; ++t)
			{
				const unsigned int j = i + offset + t;
				seqFound	= seqFound && (j < str.size() && str[j] == L'\t');
			}
			
			if (seqFound)
				i += offset + numTabs;
			else
			{
				res.push_back(str[i]);
				++i;
			}
		}
	}
}

/*static*/
unsigned int QMinkoEffectEditor::countLeftmostExtraTabs(const std::string& str)
{
	int				minNumTabs	= -1;
	unsigned int	i			= 0;
	while (i < str.size())
	{
		bool actualLine	= false;
		while (i < str.size() && (str[i] == L'\r' || str[i] == L'\n'))
			++i;

		int numTabs = 0;
		while (i < str.size() && str[i++] == L'\t')
			++numTabs;

		while (i < str.size())
		{
			const bool doBreak = str[i] == L'\n';
			if (doBreak)
				break;
			else if (isgraph(str[i]))
				actualLine	= true;
			++i;
		}

		if (actualLine && (minNumTabs < 0 || numTabs < minNumTabs))
			minNumTabs	= numTabs;
	}
	return minNumTabs < 0 ? 0 : minNumTabs;
}

/*static*/
void QMinkoEffectEditor::escapeSpecialCharacters(const std::string& str, 
												std::string& res)
{
	res.clear();
	res.reserve(str.size());
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		switch(*it)
		{
		case L'\'':
			res.append("\\\'");
			break;
		case L'\"':
			res.append("\\\"");
			break;
		case L'\?':
			res.append("\\\?");
			break;
		case L'\\':
			res.append("\\\\");
			break;
		case L'\0':
			res.append("\\0");
			break;
		case L'\a':
			res.append("\\a");
			break;
		case L'\b':
			res.append("\\b");
			break;
		case L'\f':
			res.append("\\f");
			break;
		case L'\n':
			res.append("\\n");
			break;
		case L'\r':
			res.append("\\r");
			break;
		case L'\t':
			res.append("\\t");
			break;
		case L'\v':
			res.append("\\v");
			break;
		default:
			res.push_back(*it);
			break;
		}
	}
}
#include "newtoplevelstyle.h"
#include "ui_newtoplevelstyle.h"

#include <QPushButton>

NewStopLeveStyle::NewStopLeveStyle(Project *project, QWidget *parent):
	QDialog(parent),
	ui(new Ui::NewStopLeveStyle)
{
	ui->setupUi(this);

	SQLite::Database* db = project->renderDatabase();

	for (auto data : project->dataSources())
	{
		QString name = data->dataName();
		if (name == data->primarySourceName())
			ui->dataSource->setCurrentIndex(ui->dataSource->count());
		ui->dataSource->addItem(name);
	}

	SQLite::Statement query(*db, "select key, count(key) as freq from entityKV group by key order by freq desc");

	while (query.executeStep())
	{
		const char* key = query.getColumn(0);
		int         count = query.getColumn(1);

		keys_.push_back(QString(key));
		ui->keyList->addItem(key);
	}

	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	ui->styleType->addItem(tr("Label"), StyleLayerType::ST_LABEL);
	ui->styleType->addItem(tr("Point"),StyleLayerType::ST_POINT);
	ui->styleType->addItem(tr("Line"), StyleLayerType::ST_LINE);
	ui->styleType->addItem(tr("Area"), StyleLayerType::ST_AREA);
}

NewStopLeveStyle::~NewStopLeveStyle()
{
}

QString NewStopLeveStyle::styleKey()
{
	return ui->key->text();
}

StyleLayerType NewStopLeveStyle::styleType()
{
	return (StyleLayerType )ui->styleType->currentData().toInt();
}

QString NewStopLeveStyle::dataSource()
{
	return ui->dataSource->currentText();
}

void NewStopLeveStyle::on_key_textEdited(const QString &text)
{
	ui->keyList->reset();
	ui->keyList->clear();

	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	for (QString key : keys_)
	{
		if ( key.indexOf( text) == 0)
			ui->keyList->addItem(key);
	}

	for (QString key : keys_)
	{
		if (key.indexOf(text) > 0)
			ui->keyList->addItem(key);
	}

	for (QString key : keys_)
	{
		if (key == text)
		{
			ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
			break;
		}
	}
}

void NewStopLeveStyle::on_keyList_itemSelectionChanged()
{
	QListWidgetItem *item = ui->keyList->currentItem();

	if (item)
	{
		ui->key->setText(item->text());
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	}
}

void NewStopLeveStyle::on_keyList_itemDoubleClicked(QListWidgetItem *item)
{
	on_keyList_itemSelectionChanged();
	accept();
}





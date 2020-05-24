#include "selectvalueeditdialog.h"
#include "ui_selectvalueeditdialog.h"

#include <QDesktopServices>
#include <QUrl>

SelectValueEditDialog::SelectValueEditDialog(SQLite::Database *db, const QString &dataSource, QWidget *parent) :
QDialog(parent),
ui(new Ui::SelectValueEditDialog)
{
	ui->setupUi(this);

	db_ = db;
	dataSource_ = dataSource;
	surpressSelectChangeSignals_ = false;

	SQLite::Statement query(*db_, "select key, count(key) as freq from entityKV, entity where entity.source = ? and entity.id = entityKV.id group by key order by freq desc");
	query.bind(1, dataSource_.toStdString());

	while (query.executeStep())
	{
		const char* key = query.getColumn(0);
		keys_.push_back(key);
	}

	updateKeyLists();
}

SelectValueEditDialog::~SelectValueEditDialog()
{
}

void SelectValueEditDialog::SetSelections(const QString &key, std::vector<QString> &values, bool allowKeyChange)
{
	ui->key->setText(key);

	ui->keySearchList->clear();

	selectedValues_ = values;
	ui->key->setReadOnly( !allowKeyChange);
	ui->keySearchList->setEnabled(allowKeyChange);

	updateKeyLists();
	updateValueListFull();
	updateValueList();
}

void SelectValueEditDialog::GetSelections(QString *key, std::vector<QString> *values)
{
	*key = ui->key->text();
	*values = selectedValues_;
}

void SelectValueEditDialog::on_keySearch_textEdited(const QString &text)
{
	updateKeyLists();
}

void SelectValueEditDialog::on_valueSearch_textEdited(const QString &text)
{
	updateValueList();
}

void SelectValueEditDialog::on_keySearchList_itemClicked(QListWidgetItem *item)
{
	if (ui->key->isEnabled())
	{
		ui->key->setText(item->text());
		updateValueListFull();
		updateValueList();
	}
}

void SelectValueEditDialog::updateKeyLists()
{
	ui->keySearchList->clear();

	QString keySearch = ui->keySearch->text();

	for (QString key : keys_)
	{
		bool selectIt = false;

		if (key == ui->key->text())
			selectIt = true;

		if (keySearch.isEmpty() == false)
		{
			if (key.indexOf(keySearch,0, Qt::CaseInsensitive) >= 0 || selectIt)
				ui->keySearchList->addItem(key);
		}
		else
		{
			ui->keySearchList->addItem(key);
		}

		if (selectIt)
			ui->keySearchList->setCurrentRow(ui->keySearchList->count() - 1);
	}
}

void SelectValueEditDialog::updateValueListFull()
{
	SQLite::Statement query(*db_, "select value, count(value) as freq from entityKV, entity where entity.source = ? and entity.id = entityKV.id and entityKV.key = ? group by value order by freq desc");
	query.bind(1, dataSource_.toStdString());
	query.bind(2, ui->key->text().toStdString());

	values_.clear();
	values_.push_back("*");

	while (query.executeStep())
	{
		const char* value = query.getColumn(0);
		values_.push_back(value);
	}
}

void SelectValueEditDialog::on_keyHelp_clicked()
{
	QString helpURL = QString("https://wiki.openstreetmap.org/wiki/Key:%0").arg(ui->key->text());
	QDesktopServices::openUrl(QUrl(helpURL));
}


void SelectValueEditDialog::on_valueList_itemSelectionChanged()
{
	if (surpressSelectChangeSignals_ == false)
	{
		surpressSelectChangeSignals_ = true;

		std::vector<QString> newSelectedList;
		for (int row = 0; row < ui->valueList->count(); ++row)
		{
			if (ui->valueList->item(row)->isSelected())
				newSelectedList.push_back(ui->valueList->item(row)->text());
		}

		if (newSelectedList.size() > 1 && newSelectedList[0] == "*"  && selectedValues_.size() > 0 && selectedValues_[0] != "*")
		{
			// didn't have * before, added it, clear out other selection
			ui->valueList->clearSelection();
			ui->valueList->item(0)->setSelected(true);
			newSelectedList.erase(newSelectedList.begin() + 1, newSelectedList.end());
		}

		if (newSelectedList.size() > 1 && newSelectedList[0] == "*"  && selectedValues_.size() > 0 && selectedValues_[0] == "*")
		{
			// did * before, added new item, clear out *
			ui->valueList->item(0)->setSelected(false);
			newSelectedList.erase(newSelectedList.begin());
		}

		selectedValues_ = newSelectedList;

		surpressSelectChangeSignals_ = false;
	}
}

void SelectValueEditDialog::updateValueList()
{
	surpressSelectChangeSignals_ = true;
	ui->valueList->clear();

	QString valueSearch = ui->valueSearch->text();

	for (QString value : values_)
	{
		bool selected = false;

		if (std::find(selectedValues_.begin(), selectedValues_.end(), value) != selectedValues_.end())
			selected = true;

		if (valueSearch.isEmpty() == false)
		{
			if (value.indexOf(valueSearch, 0, Qt::CaseInsensitive) >= 0 || selected)
				ui->valueList->addItem(value);
		}
		else
		{
			ui->valueList->addItem(value);
		}

		if (selected)
			ui->valueList->item(ui->valueList->count() - 1)->setSelected(true);
	}

	surpressSelectChangeSignals_ = false;
}


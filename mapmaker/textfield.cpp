#include "textfield.h"

void TextFieldProcessor::RequiredKeys(const QString &field, std::set<QString> *keys)
{
	bool inField = false;
	QString key;

	for (int index = 0; index < field.count(); ++index)
	{
		if (inField == false)
		{
			if (field[index] == '[' && (index == 0 || field[index - 1] != '\\'))
			{
				inField = true;
			}
		}
		else
		{
			if (field[index] == ']')
			{
				keys->insert(key.trimmed());
				key.clear();
				inField = false;
			}
			else if (field[index] == ',')
			{
				keys->insert(key.trimmed());
				key.clear();
				// keep scanning.
			}
			else
			{
				key.append(field[index]);
			}
		}
	}
}

QString TextFieldProcessor::Expand(const QString field, std::map<QString, QString> keys)
{
	bool inField = false;
	QString key;
	QString ret;

	for (int index = 0; index < field.count(); ++index)
	{
		if (inField == false)
		{
			if (field[index] == '[' && (index == 0 || field[index - 1] != '\\'))
			{
				inField = true;
			}
			else
			{
				ret += field[index];
			}
		}
		else
		{
			if (field[index] == ']')
			{
				key = key.trimmed();
				auto i = keys.find(key);
				if (i != keys.end())
					ret += i->second;

				key.clear();
				inField = false;
			}
			else if (field[index] == ',')
			{
				key = key.trimmed();
				auto i = keys.find(key);
				if (i != keys.end())
				{
					ret += i->second;

					// skip the rest of the field
					while (field[index] != ']' && index < field.count())
					{
						++index;
					}
					--index;
				}

				key.clear();
			}
			else
			{
				key.append(field[index]);
			}
		}
	}

	return ret;
}







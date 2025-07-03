#pragma once

#include <QString>
#include <set>
#include <map>

/// Utility functions for expanding label text fields.
class TextFieldProcessor
{
public:
	static void RequiredKeys(const QString &field, std::set<QString> *keys);
	static QString Expand(const QString field, std::map<QString, QString> keys);

};






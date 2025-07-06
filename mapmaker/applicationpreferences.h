#pragma once

#include <QStringList>
#include <filesystem>

/// Application-wide preferences stored under the user's application data
/// directory. Currently manages the list of recently opened projects but is
/// intended to centralize other persisted settings in the future.
class ApplicationPreferences {
public:
    /// Directory where the MRU file resides. Respects the optional
    /// OSMMAPMAKER_APPDATA environment override.
    static QString appDataDir();

    /// Location of the MRU file on disk.
    static std::filesystem::path mruFilePath();

    /// Read the MRU list from disk. Missing files return an empty list.
    static QStringList readMRU();

    /// Save the provided project paths to disk.
    static void saveMRU(const QStringList& paths);

    /// Add a path to the MRU list, keeping only the newest MAX_ENTRIES_MRU items.
    static void addProjectToMRU(const QString& projectPath);

    /// Return the first project in the MRU list that still exists on disk.
    static QString mostRecentExistingMRU();

    static constexpr int MAX_ENTRIES_MRU = 15; ///< Maximum number of paths stored
};

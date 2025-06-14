/*
 * OB-Xf - a continuation of the last open source version of OB-Xd.
 *
 * OB-Xd was originally written by Vadim Filatov, and then a version
 * was released under the GPL3 at https://github.com/reales/OB-Xd.
 * Subsequently, the product was continued by DiscoDSP and the copyright
 * holders as an excellent closed source product. For more info,
 * see "HISTORY.md" in the root of this repository.
 *
 * This repository is a successor to the last open source release,
 * a version marked as 2.11. Copyright 2013-2025 by the authors
 * as indicated in the original release, and subsequent authors
 * per the GitHub transaction log.
 *
 * OB-Xf is released under the GNU General Public Licence v3 or later
 * (GPL-3.0-or-later). The license is found in the file "LICENSE"
 * in the root of this repository or at:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Source code is available at https://github.com/surge-synthesizer/OB-Xf
 */

#ifndef OBXF_SRC_UTILS_H
#define OBXF_SRC_UTILS_H

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "Constants.h"

inline static float getPitch(float index) { return 440.f * expf(mult * index); };

inline static float linsc(float param, const float min, const float max)
{
    return (param) * (max - min) + min;
}

inline static float logsc(float param, const float min, const float max, const float rolloff = 19.f)
{
    return ((expf(param * logf(rolloff + 1.f)) - 1.f) / (rolloff)) * (max - min) + min;
}

class Utils final
{
  public:
    Utils();

    ~Utils();

    // File System Methods
    [[nodiscard]] juce::File getDocumentFolder() const;

    [[nodiscard]] juce::File getMidiFolder() const;

    [[nodiscard]] juce::File getSkinFolder() const;

    [[nodiscard]] juce::File getBanksFolder() const;

    // Skin Management
    [[nodiscard]] const juce::Array<juce::File> &getSkinFiles() const;

    [[nodiscard]] juce::File getCurrentSkinFolder() const;

    void setCurrentSkinFolder(const juce::String &folderName);

    void scanAndUpdateSkins();

    // Bank Management
    [[nodiscard]] const juce::Array<juce::File> &getBankFiles() const;

    [[nodiscard]] juce::File getCurrentBankFile() const;

    // GUI Settings
    void setGuiSize(int size);

    [[nodiscard]] int getGuiSize() const { return gui_size; }
    [[nodiscard]] float getPixelScaleFactor() const { return physicalPixelScaleFactor; }
    void setPixelScaleFactor(const float factor) { physicalPixelScaleFactor = factor; }

    // FXB
    bool loadFromFXBFile(const juce::File &fxbFile);

    void scanAndUpdateBanks();

    // banks
    bool deleteBank();

    void saveBank() const;

    bool saveBank(const juce::File &fxbFile);

    [[nodiscard]] bool saveFXBFile(const juce::File &fxbFile) const;

    [[nodiscard]] juce::String getCurrentBank() const { return currentBank; }

    // presets
    [[nodiscard]] juce::String getCurrentPreset() const { return currentPreset; }

    bool saveFXPFile(const juce::File &fxpFile) const;

    bool loadPreset(const juce::File &fxpFile);

    bool savePreset(const juce::File &fxpFile);

    void savePreset();

    void serializePreset(juce::MemoryBlock &memoryBlock) const;

    void changePresetName(const juce::String &name) const;

    void newPreset(const juce::String &name) const;

    void initializePreset() const;

    bool loadFromFXPFile(const juce::File &fxpFile);

    juce::File getPresetsFolder() const { return getDocumentFolder().getChildFile("Presets"); }

    bool isMemoryBlockAPreset(const juce::MemoryBlock &mb);

    // should refactor all callbacks to be like this? or not? :-)
    using HostUpdateCallback = std::function<void()>;
    void setHostUpdateCallback(HostUpdateCallback callback)
    {
        hostUpdateCallback = std::move(callback);
    }

    // callbacks
    std::function<bool(juce::MemoryBlock &)> loadMemoryBlockCallback;
    std::function<void(juce::MemoryBlock &)> getStateInformationCallback;
    std::function<int()> getNumProgramsCallback;
    std::function<void(juce::MemoryBlock &)> getCurrentProgramStateInformation;
    std::function<int()> getNumPrograms;
    std::function<void(char *, int)> copyProgramNameToBuffer;
    std::function<void(const juce::String &)> setProgramName;
    std::function<void()> resetProgramToDefault;
    std::function<void()> sendChangeMessage;
    std::function<void(int)> setCurrentProgram;
    std::function<bool(int, const juce::String &)> isProgramNameCallback;

  private:
    // Config Management
    std::unique_ptr<juce::PropertiesFile> config;
    juce::InterProcessLock configLock;

    void updateConfig();

    // File Collections
    juce::Array<juce::File> skinFiles;
    juce::Array<juce::File> bankFiles;

    // Current States
    juce::String currentSkin;
    juce::String currentBank;
    int gui_size{};
    float physicalPixelScaleFactor{};

    juce::File currentBankFile;
    HostUpdateCallback hostUpdateCallback;

    // preset
    juce::String currentPreset;
    juce::File currentPresetFile;
};

#endif // OBXF_SRC_UTILS_H

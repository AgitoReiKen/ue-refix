// Alexander (AgitoReiKen) Moskalenko (C) 2022
#include "Themes/Default/DefaultTheme.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "RestyleProcessor.h"

#include "Themes/Default/NodeRestyleDefault.h"
#include "Themes/Default/PinRestyleDefault.h"
#include "Themes/Default/WireRestyleDefault.h"

FRestyleDefaultTheme::~FRestyleDefaultTheme()
{
}

FRestyleDefaultTheme::FRestyleDefaultTheme()
{
	NodeFactoryProvider = MakeShared<FNodeRestyleDefault>();
	PinFactoryProvider = MakeShared<FPinRestyleDefault>();
	WireFactoryProvider = MakeShared<FWireRestyleDefault>();
}

FName FRestyleDefaultTheme::GetId()
{
	return TEXT("Default");
}

/*
 * void FRestyleModule::RegisterSettings()
{
	Settings = GetMutableDefault<URestyleSettings>();
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		auto SettingsSection = SettingsModule->RegisterSettings(
			"Editor", "Plugins", "Restyle",
			FText::FromString("Restyle"),
			FText::FromString(
				"Changed elements, must be reconstructed again to apply changes (if ApplyOnChange is enabled). E.g. reopen widgets"),
			Settings);
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FRestyleModule::OnSettingsChanged);
		}
	}
}

void FRestyleModule::UnregisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Restyle");
	}
}
 */
void FRestyleDefaultTheme::Register()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		auto SettingsSection = SettingsModule->RegisterSettings(
			"Editor", "Plugins", "RestyleDefault",
			FText::FromString("Restyle [Default]"),
			FText::FromString(""),
			GetMutableDefault<UDefaultThemeSettings>());
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnSave().BindRaw(this,
			                                      &FRestyleDefaultTheme::OnSettingsChanged);
		}
	}
}

void FRestyleDefaultTheme::Unregister()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Editor", "Plugins", "RestyleDefault");
	}
}

TSharedPtr<ISubjectRestyleInterface> FRestyleDefaultTheme::GetSubjectFactoryProvider(ERestyleSubject ForSubject)
{
	switch (ForSubject)
	{
	case ERestyleSubject::Node: return NodeFactoryProvider;
	case ERestyleSubject::Pin: return PinFactoryProvider; // PinFactoryProvider;
	case ERestyleSubject::PinConnection: return WireFactoryProvider; // WireFactoryProvider;
	default: return nullptr;
	}
}

bool FRestyleDefaultTheme::OnSettingsChanged()
{
	if (NodeFactoryProvider->IsRegistered())
	{
		NodeFactoryProvider->Update();
	}
	if (PinFactoryProvider->IsRegistered())
	{
		PinFactoryProvider->Update();
	}
	auto Mutable = GetMutableDefault<UDefaultThemeSettings>();
	Mutable->SaveConfig(CPF_Config, *Mutable->GetGlobalUserConfigFilename());
	return true;
}
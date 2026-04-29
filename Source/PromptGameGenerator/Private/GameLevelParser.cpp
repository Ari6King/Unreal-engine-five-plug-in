// Copyright 2024 PromptGameGenerator Contributors. All Rights Reserved.

#include "GameLevelParser.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FGameLevelParser::ParseLevelSpec(const FString& JsonString, FGameLevelSpec& OutSpec)
{
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("PromptGameGenerator: Failed to parse level JSON."));
		return false;
	}

	OutSpec.LevelName = RootObject->GetStringField(TEXT("level_name"));
	OutSpec.Description = RootObject->GetStringField(TEXT("description"));

	const TSharedPtr<FJsonObject>* EnvironmentObj;
	if (RootObject->TryGetObjectField(TEXT("environment"), EnvironmentObj))
	{
		OutSpec.Environment = ParseEnvironment(*EnvironmentObj);
	}

	const TSharedPtr<FJsonObject>* TerrainObj;
	if (RootObject->TryGetObjectField(TEXT("terrain"), TerrainObj))
	{
		OutSpec.Terrain = ParseTerrain(*TerrainObj);
	}

	const TSharedPtr<FJsonObject>* LightingObj;
	if (RootObject->TryGetObjectField(TEXT("lighting"), LightingObj))
	{
		OutSpec.Lighting = ParseLighting(*LightingObj);
	}

	const TArray<TSharedPtr<FJsonValue>>* ActorsArray;
	if (RootObject->TryGetArrayField(TEXT("actors"), ActorsArray))
	{
		for (const auto& ActorValue : *ActorsArray)
		{
			TSharedPtr<FJsonObject> ActorObj = ActorValue->AsObject();
			if (ActorObj.IsValid())
			{
				OutSpec.Actors.Add(ParseActor(ActorObj));
			}
		}
	}

	const TSharedPtr<FJsonObject>* GameplayObj;
	if (RootObject->TryGetObjectField(TEXT("gameplay"), GameplayObj))
	{
		OutSpec.Gameplay = ParseGameplay(*GameplayObj);
	}

	const TSharedPtr<FJsonObject>* PostProcessObj;
	if (RootObject->TryGetObjectField(TEXT("post_processing"), PostProcessObj))
	{
		OutSpec.PostProcessing = ParsePostProcessing(*PostProcessObj);
	}

	const TSharedPtr<FJsonObject>* AudioObj;
	if (RootObject->TryGetObjectField(TEXT("audio"), AudioObj))
	{
		OutSpec.Audio = ParseAudio(*AudioObj);
	}

	UE_LOG(LogTemp, Log, TEXT("PromptGameGenerator: Parsed level '%s' with %d actors."),
		*OutSpec.LevelName, OutSpec.Actors.Num());

	return true;
}

FColorSpec FGameLevelParser::ParseColor(const TSharedPtr<FJsonObject>& Obj)
{
	FColorSpec Color;
	if (Obj.IsValid())
	{
		Obj->TryGetNumberField(TEXT("r"), Color.R);
		Obj->TryGetNumberField(TEXT("g"), Color.G);
		Obj->TryGetNumberField(TEXT("b"), Color.B);
	}
	return Color;
}

FPositionSpec FGameLevelParser::ParsePosition(const TSharedPtr<FJsonObject>& Obj)
{
	FPositionSpec Pos;
	if (Obj.IsValid())
	{
		double X, Y, Z;
		if (Obj->TryGetNumberField(TEXT("x"), X)) Pos.X = static_cast<float>(X);
		if (Obj->TryGetNumberField(TEXT("y"), Y)) Pos.Y = static_cast<float>(Y);
		if (Obj->TryGetNumberField(TEXT("z"), Z)) Pos.Z = static_cast<float>(Z);
	}
	return Pos;
}

FRotationSpec FGameLevelParser::ParseRotation(const TSharedPtr<FJsonObject>& Obj)
{
	FRotationSpec Rot;
	if (Obj.IsValid())
	{
		double Pitch, Yaw, Roll;
		if (Obj->TryGetNumberField(TEXT("pitch"), Pitch)) Rot.Pitch = static_cast<float>(Pitch);
		if (Obj->TryGetNumberField(TEXT("yaw"), Yaw)) Rot.Yaw = static_cast<float>(Yaw);
		if (Obj->TryGetNumberField(TEXT("roll"), Roll)) Rot.Roll = static_cast<float>(Roll);
	}
	return Rot;
}

FMaterialSpec FGameLevelParser::ParseMaterial(const TSharedPtr<FJsonObject>& Obj)
{
	FMaterialSpec Mat;
	if (Obj.IsValid())
	{
		const TSharedPtr<FJsonObject>* ColorObj;
		if (Obj->TryGetObjectField(TEXT("color"), ColorObj))
		{
			Mat.Color = ParseColor(*ColorObj);
		}
		double Metallic, Roughness, Emissive;
		if (Obj->TryGetNumberField(TEXT("metallic"), Metallic)) Mat.Metallic = static_cast<float>(Metallic);
		if (Obj->TryGetNumberField(TEXT("roughness"), Roughness)) Mat.Roughness = static_cast<float>(Roughness);
		if (Obj->TryGetNumberField(TEXT("emissive_strength"), Emissive)) Mat.EmissiveStrength = static_cast<float>(Emissive);
	}
	return Mat;
}

FActorSpec FGameLevelParser::ParseActor(const TSharedPtr<FJsonObject>& Obj)
{
	FActorSpec Actor;
	if (!Obj.IsValid()) return Actor;

	Actor.Type = Obj->GetStringField(TEXT("type"));
	Actor.Name = Obj->GetStringField(TEXT("name"));

	const TSharedPtr<FJsonObject>* PosObj;
	if (Obj->TryGetObjectField(TEXT("position"), PosObj))
		Actor.Position = ParsePosition(*PosObj);

	const TSharedPtr<FJsonObject>* RotObj;
	if (Obj->TryGetObjectField(TEXT("rotation"), RotObj))
		Actor.Rotation = ParseRotation(*RotObj);

	const TSharedPtr<FJsonObject>* ScaleObj;
	if (Obj->TryGetObjectField(TEXT("scale"), ScaleObj))
	{
		Actor.Scale = ParsePosition(*ScaleObj);
	}
	else
	{
		Actor.Scale.X = 1.0f;
		Actor.Scale.Y = 1.0f;
		Actor.Scale.Z = 1.0f;
	}

	const TSharedPtr<FJsonObject>* MatObj;
	if (Obj->TryGetObjectField(TEXT("material"), MatObj))
		Actor.Material = ParseMaterial(*MatObj);

	Obj->TryGetBoolField(TEXT("physics_enabled"), Actor.bPhysicsEnabled);

	FString CustomMesh;
	if (Obj->TryGetStringField(TEXT("custom_mesh"), CustomMesh))
	{
		Actor.CustomMesh = CustomMesh;
	}

	const TArray<TSharedPtr<FJsonValue>>* TagsArray;
	if (Obj->TryGetArrayField(TEXT("tags"), TagsArray))
	{
		for (const auto& TagValue : *TagsArray)
		{
			Actor.Tags.Add(TagValue->AsString());
		}
	}

	return Actor;
}

FEnvironmentSpec FGameLevelParser::ParseEnvironment(const TSharedPtr<FJsonObject>& Obj)
{
	FEnvironmentSpec Env;
	if (!Obj.IsValid()) return Env;

	Env.Type = Obj->GetStringField(TEXT("type"));
	Env.TimeOfDay = Obj->GetStringField(TEXT("time_of_day"));
	Env.Weather = Obj->GetStringField(TEXT("weather"));

	const TSharedPtr<FJsonObject>* SkyColorObj;
	if (Obj->TryGetObjectField(TEXT("sky_color"), SkyColorObj))
		Env.SkyColor = ParseColor(*SkyColorObj);

	double AmbientIntensity, FogDensity;
	if (Obj->TryGetNumberField(TEXT("ambient_intensity"), AmbientIntensity))
		Env.AmbientIntensity = static_cast<float>(AmbientIntensity);
	if (Obj->TryGetNumberField(TEXT("fog_density"), FogDensity))
		Env.FogDensity = static_cast<float>(FogDensity);

	return Env;
}

FTerrainSpec FGameLevelParser::ParseTerrain(const TSharedPtr<FJsonObject>& Obj)
{
	FTerrainSpec Terrain;
	if (!Obj.IsValid()) return Terrain;

	Obj->TryGetBoolField(TEXT("enabled"), Terrain.bEnabled);

	double SizeX, SizeY, HeightScale, NoiseScale;
	if (Obj->TryGetNumberField(TEXT("size_x"), SizeX)) Terrain.SizeX = static_cast<float>(SizeX);
	if (Obj->TryGetNumberField(TEXT("size_y"), SizeY)) Terrain.SizeY = static_cast<float>(SizeY);
	if (Obj->TryGetNumberField(TEXT("height_scale"), HeightScale)) Terrain.HeightScale = static_cast<float>(HeightScale);
	if (Obj->TryGetNumberField(TEXT("noise_scale"), NoiseScale)) Terrain.NoiseScale = static_cast<float>(NoiseScale);

	double NoiseOctaves;
	if (Obj->TryGetNumberField(TEXT("noise_octaves"), NoiseOctaves))
		Terrain.NoiseOctaves = static_cast<int32>(NoiseOctaves);

	Terrain.MaterialType = Obj->GetStringField(TEXT("material"));

	const TSharedPtr<FJsonObject>* ColorObj;
	if (Obj->TryGetObjectField(TEXT("color"), ColorObj))
		Terrain.Color = ParseColor(*ColorObj);

	return Terrain;
}

FLightingSpec FGameLevelParser::ParseLighting(const TSharedPtr<FJsonObject>& Obj)
{
	FLightingSpec Lighting;
	if (!Obj.IsValid()) return Lighting;

	const TSharedPtr<FJsonObject>* DirLightObj;
	if (Obj->TryGetObjectField(TEXT("directional_light"), DirLightObj))
	{
		double Intensity;
		if ((*DirLightObj)->TryGetNumberField(TEXT("intensity"), Intensity))
			Lighting.DirectionalLight.Intensity = static_cast<float>(Intensity);

		const TSharedPtr<FJsonObject>* ColorObj;
		if ((*DirLightObj)->TryGetObjectField(TEXT("color"), ColorObj))
			Lighting.DirectionalLight.Color = ParseColor(*ColorObj);

		const TSharedPtr<FJsonObject>* RotObj;
		if ((*DirLightObj)->TryGetObjectField(TEXT("rotation"), RotObj))
			Lighting.DirectionalLight.Rotation = ParseRotation(*RotObj);
	}

	const TSharedPtr<FJsonObject>* SkyLightObj;
	if (Obj->TryGetObjectField(TEXT("sky_light"), SkyLightObj))
	{
		double Intensity;
		if ((*SkyLightObj)->TryGetNumberField(TEXT("intensity"), Intensity))
			Lighting.SkyLight.Intensity = static_cast<float>(Intensity);

		const TSharedPtr<FJsonObject>* ColorObj;
		if ((*SkyLightObj)->TryGetObjectField(TEXT("color"), ColorObj))
			Lighting.SkyLight.Color = ParseColor(*ColorObj);
	}

	const TArray<TSharedPtr<FJsonValue>>* PointLightsArray;
	if (Obj->TryGetArrayField(TEXT("point_lights"), PointLightsArray))
	{
		for (const auto& LightValue : *PointLightsArray)
		{
			TSharedPtr<FJsonObject> LightObj = LightValue->AsObject();
			if (!LightObj.IsValid()) continue;

			FPointLightSpec PointLight;

			const TSharedPtr<FJsonObject>* PosObj;
			if (LightObj->TryGetObjectField(TEXT("position"), PosObj))
				PointLight.Position = ParsePosition(*PosObj);

			double Intensity, Attenuation;
			if (LightObj->TryGetNumberField(TEXT("intensity"), Intensity))
				PointLight.Intensity = static_cast<float>(Intensity);
			if (LightObj->TryGetNumberField(TEXT("attenuation_radius"), Attenuation))
				PointLight.AttenuationRadius = static_cast<float>(Attenuation);

			const TSharedPtr<FJsonObject>* ColorObj;
			if (LightObj->TryGetObjectField(TEXT("color"), ColorObj))
				PointLight.Color = ParseColor(*ColorObj);

			Lighting.PointLights.Add(PointLight);
		}
	}

	return Lighting;
}

FGameplaySpec FGameLevelParser::ParseGameplay(const TSharedPtr<FJsonObject>& Obj)
{
	FGameplaySpec Gameplay;
	if (!Obj.IsValid()) return Gameplay;

	const TSharedPtr<FJsonObject>* PlayerStartObj;
	if (Obj->TryGetObjectField(TEXT("player_start"), PlayerStartObj))
		Gameplay.PlayerStart = ParsePosition(*PlayerStartObj);

	Gameplay.GameMode = Obj->GetStringField(TEXT("game_mode"));

	const TArray<TSharedPtr<FJsonValue>>* ObjectivesArray;
	if (Obj->TryGetArrayField(TEXT("objectives"), ObjectivesArray))
	{
		for (const auto& ObjValue : *ObjectivesArray)
		{
			TSharedPtr<FJsonObject> ObjObj = ObjValue->AsObject();
			if (!ObjObj.IsValid()) continue;

			FObjectiveSpec Objective;
			Objective.Type = ObjObj->GetStringField(TEXT("type"));
			Objective.Description = ObjObj->GetStringField(TEXT("description"));

			double TargetCount;
			if (ObjObj->TryGetNumberField(TEXT("target_count"), TargetCount))
				Objective.TargetCount = static_cast<int32>(TargetCount);

			Gameplay.Objectives.Add(Objective);
		}
	}

	return Gameplay;
}

FPostProcessingSpec FGameLevelParser::ParsePostProcessing(const TSharedPtr<FJsonObject>& Obj)
{
	FPostProcessingSpec PP;
	if (!Obj.IsValid()) return PP;

	double Val;
	if (Obj->TryGetNumberField(TEXT("bloom_intensity"), Val)) PP.BloomIntensity = static_cast<float>(Val);
	if (Obj->TryGetNumberField(TEXT("auto_exposure_min"), Val)) PP.AutoExposureMin = static_cast<float>(Val);
	if (Obj->TryGetNumberField(TEXT("auto_exposure_max"), Val)) PP.AutoExposureMax = static_cast<float>(Val);
	if (Obj->TryGetNumberField(TEXT("vignette_intensity"), Val)) PP.VignetteIntensity = static_cast<float>(Val);
	if (Obj->TryGetNumberField(TEXT("color_saturation"), Val)) PP.ColorSaturation = static_cast<float>(Val);
	if (Obj->TryGetNumberField(TEXT("color_contrast"), Val)) PP.ColorContrast = static_cast<float>(Val);
	if (Obj->TryGetNumberField(TEXT("ambient_occlusion_intensity"), Val)) PP.AmbientOcclusionIntensity = static_cast<float>(Val);

	return PP;
}

FAudioSpec FGameLevelParser::ParseAudio(const TSharedPtr<FJsonObject>& Obj)
{
	FAudioSpec Audio;
	if (!Obj.IsValid()) return Audio;

	Audio.AmbientSound = Obj->GetStringField(TEXT("ambient_sound"));
	Audio.MusicMood = Obj->GetStringField(TEXT("music_mood"));

	return Audio;
}

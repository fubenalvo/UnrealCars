#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GroundSurfaceSoundComponent.generated.h"

class UAudioComponent;

USTRUCT(BlueprintType)
struct FGroundSurfaceSoundEntry
{
    GENERATED_BODY()

    /** Physical Material associated with this ground sound. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Sound")
    TObjectPtr<UPhysicalMaterial> PhysicalMaterial = nullptr;

    /** Looping sound used for this Physical Material. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Sound")
    TObjectPtr<USoundBase> Sound = nullptr;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class UNREALCARS_API UGroundSurfaceSoundComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UGroundSurfaceSoundComponent();

    /**
     * Updates the ground sound based on vehicle speed and the
     * Physical Material detected underneath the vehicle.
     */
    UFUNCTION(BlueprintCallable, Category = "Ground Sound")
    void UpdateGroundSound(float Speed);

    /** Returns the currently detected Physical Material. */
    UFUNCTION(BlueprintPure, Category = "Ground Sound")
    UPhysicalMaterial* GetCurrentPhysicalMaterial() const;

    /** Returns the currently active ground sound. */
    UFUNCTION(BlueprintPure, Category = "Ground Sound")
    USoundBase* GetCurrentSound() const;

    /** Returns the current ground sound volume. */
    UFUNCTION(BlueprintPure, Category = "Ground Sound")
    float GetGroundVolume() const;

protected:

    virtual void BeginPlay() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason) override;

private:

    /** Physical Material to sound mappings. */
    UPROPERTY(EditAnywhere, Category = "Ground Sound")
    TArray<FGroundSurfaceSoundEntry> SurfaceSounds;

    /**
     * Physical Material reported when no valid surface Physical Material
     * can be detected.
     *
     * This is informational and can also be used by gameplay code.
     */
    UPROPERTY(EditAnywhere, Category = "Ground Sound")
    TObjectPtr<UPhysicalMaterial> FallbackPhysicalMaterial = nullptr;

    /**
     * Sound played when no valid Physical Material sound can be found.
     *
     * This is intentionally independent from SurfaceSounds.
     */
    UPROPERTY(EditAnywhere, Category = "Ground Sound")
    TObjectPtr<USoundBase> FallbackSound = nullptr;

    /** Maximum distance of the downward ground trace in centimeters. */
    UPROPERTY(EditAnywhere, Category = "Ground Sound", meta = (ClampMin = "1.0"))
    float TraceDistance = 150.0f;

    /** Speed at which the ground sound reaches maximum volume. */
    UPROPERTY(EditAnywhere, Category = "Ground Sound", meta = (ClampMin = "1.0"))
    float MaxSpeed = 160.0f;

    /** Crossfade duration in seconds. */
    UPROPERTY(EditAnywhere, Category = "Ground Sound", meta = (ClampMin = "0.01"))
    float CrossfadeDuration = 0.25f;

    /** Maximum ground sound volume. */
    UPROPERTY(EditAnywhere, Category = "Ground Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MaxVolume = 1.0f;

    /** Collision channel used for the ground trace. */
    UPROPERTY(EditAnywhere, Category = "Ground Sound")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    /** First Audio Component used for ground sounds. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> AudioComponentA = nullptr;

    /** Second Audio Component used for ground sounds. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> AudioComponentB = nullptr;

    /** Whether Audio Component A is currently active. */
    UPROPERTY(Transient)
    bool bAudioAIsActive = true;

    /** Current Physical Material. */
    UPROPERTY(Transient)
    TObjectPtr<UPhysicalMaterial> CurrentPhysicalMaterial = nullptr;

    /** Current ground sound. */
    UPROPERTY(Transient)
    TObjectPtr<USoundBase> CurrentSound = nullptr;

    /** Current volume based on vehicle speed. */
    UPROPERTY(Transient)
    float CurrentGroundVolume = 0.0f;

    /** Current crossfade time. */
    float CrossfadeTime = 0.0f;

    /** Whether a crossfade is currently active. */
    bool bIsCrossfading = false;

    /** Volume of the old sound when the crossfade started. */
    float CrossfadeStartOldVolume = 0.0f;

    /** Target volume of the new sound. */
    float CrossfadeTargetNewVolume = 0.0f;

    /** Audio Component currently fading out. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> CrossfadeOldComponent = nullptr;

    /** Audio Component currently fading in. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> CrossfadeNewComponent = nullptr;

    /** Finds a sound for the specified Physical Material. */
    USoundBase* FindSoundForPhysicalMaterial(
        UPhysicalMaterial* PhysicalMaterial) const;

    /** Performs the downward trace. */
    UPhysicalMaterial* DetectGroundPhysicalMaterial() const;

    /** Calculates volume from vehicle speed. */
    float CalculateSpeedVolume(float Speed) const;

    /** Starts a crossfade to a new sound. */
    void StartCrossfade(USoundBase* NewSound);

    /** Updates the active crossfade. */
    void UpdateCrossfade(float DeltaTime);

    /** Returns the currently active Audio Component. */
    UAudioComponent* GetActiveAudioComponent() const;

    /** Returns the currently inactive Audio Component. */
    UAudioComponent* GetInactiveAudioComponent() const;
};
#include "GroundSurfaceSoundComponent.h"

#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"

UGroundSurfaceSoundComponent::UGroundSurfaceSoundComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGroundSurfaceSoundComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();

    if (!Owner)
    {
        return;
    }

    AudioComponentA = NewObject<UAudioComponent>(
        Owner,
        TEXT("GroundAudioComponentA"));

    if (AudioComponentA)
    {
        Owner->AddInstanceComponent(AudioComponentA);

        AudioComponentA->bAutoActivate = false;
        AudioComponentA->bIsUISound = false;

        AudioComponentA->AttachToComponent(
            Owner->GetRootComponent(),
            FAttachmentTransformRules::KeepRelativeTransform);

        AudioComponentA->RegisterComponentWithWorld(GetWorld());
    }

    AudioComponentB = NewObject<UAudioComponent>(
        Owner,
        TEXT("GroundAudioComponentB"));

    if (AudioComponentB)
    {
        Owner->AddInstanceComponent(AudioComponentB);

        AudioComponentB->bAutoActivate = false;
        AudioComponentB->bIsUISound = false;

        AudioComponentB->AttachToComponent(
            Owner->GetRootComponent(),
            FAttachmentTransformRules::KeepRelativeTransform);

        AudioComponentB->RegisterComponentWithWorld(GetWorld());
    }

    bAudioAIsActive = true;

    CurrentPhysicalMaterial = nullptr;
    CurrentSound = nullptr;
    CurrentGroundVolume = 0.0f;

    CrossfadeTime = 0.0f;
    bIsCrossfading = false;

    CrossfadeOldComponent = nullptr;
    CrossfadeNewComponent = nullptr;
}

void UGroundSurfaceSoundComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    if (AudioComponentA)
    {
        AudioComponentA->Stop();
        AudioComponentA->DestroyComponent();
        AudioComponentA = nullptr;
    }

    if (AudioComponentB)
    {
        AudioComponentB->Stop();
        AudioComponentB->DestroyComponent();
        AudioComponentB = nullptr;
    }

    CrossfadeOldComponent = nullptr;
    CrossfadeNewComponent = nullptr;
    bIsCrossfading = false;

    Super::EndPlay(EndPlayReason);
}

void UGroundSurfaceSoundComponent::UpdateGroundSound(float Speed)
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    /*
     * Update an already running crossfade.
     */
    UpdateCrossfade(GetWorld()->GetDeltaSeconds());

    /*
     * Calculate volume from current vehicle speed.
     */
    CurrentGroundVolume = CalculateSpeedVolume(Speed);

    /*
     * Try to detect the Physical Material underneath the vehicle.
     */
    UPhysicalMaterial* DetectedPhysicalMaterial =
        DetectGroundPhysicalMaterial();

    /*
     * Try to find a sound for the detected Physical Material.
     */
    USoundBase* DetectedSound = nullptr;

    if (DetectedPhysicalMaterial)
    {
        DetectedSound =
            FindSoundForPhysicalMaterial(
                DetectedPhysicalMaterial);
    }

    /*
     * No usable Physical Material sound was found.
     *
     * This includes:
     *
     * - no trace hit
     * - trace hit but no Physical Material
     * - Physical Material exists but has no configured sound
     *
     * In all of these cases use the explicit fallback.
     */
    if (!DetectedSound)
    {
        DetectedPhysicalMaterial =
            FallbackPhysicalMaterial;

        DetectedSound =
            FallbackSound;
    }

    CurrentPhysicalMaterial =
        DetectedPhysicalMaterial;

    /*
     * Even the fallback has no sound configured.
     * Stop the current ground sound.
     */
    if (!DetectedSound)
    {
        if (bIsCrossfading)
        {
            if (CrossfadeOldComponent)
            {
                CrossfadeOldComponent->Stop();
                CrossfadeOldComponent->SetVolumeMultiplier(0.0f);
            }

            if (CrossfadeNewComponent)
            {
                CrossfadeNewComponent->Stop();
                CrossfadeNewComponent->SetVolumeMultiplier(0.0f);
            }

            CrossfadeOldComponent = nullptr;
            CrossfadeNewComponent = nullptr;

            bIsCrossfading = false;
            CrossfadeTime = 0.0f;
        }

        if (UAudioComponent* ActiveComponent =
            GetActiveAudioComponent())
        {
            ActiveComponent->Stop();
            ActiveComponent->SetVolumeMultiplier(0.0f);
        }

        CurrentSound = nullptr;

        return;
    }

    /*
     * No sound is currently active.
     * Start the detected or fallback sound directly.
     */
    if (!CurrentSound)
    {
        CurrentSound = DetectedSound;

        UAudioComponent* ActiveComponent =
            GetActiveAudioComponent();

        if (!ActiveComponent)
        {
            return;
        }

        ActiveComponent->SetSound(CurrentSound);

        ActiveComponent->SetVolumeMultiplier(
            CurrentGroundVolume);

        if (CurrentGroundVolume > 0.0f &&
            !ActiveComponent->IsPlaying())
        {
            ActiveComponent->Play();
        }

        return;
    }

    /*
     * The detected sound changed.
     *
     * This also handles:
     *
     * Asphalt -> Fallback
     * Fallback -> Asphalt
     * Asphalt -> Grass
     * Grass -> Gravel
     */
    if (DetectedSound != CurrentSound)
    {
        StartCrossfade(DetectedSound);

        return;
    }

    /*
     * Same sound as before.
     * Just update its volume.
     */
    if (!bIsCrossfading)
    {
        UAudioComponent* ActiveComponent =
            GetActiveAudioComponent();

        if (ActiveComponent)
        {
            ActiveComponent->SetVolumeMultiplier(
                CurrentGroundVolume);

            if (CurrentGroundVolume > 0.0f &&
                !ActiveComponent->IsPlaying())
            {
                ActiveComponent->Play();
            }
        }
    }
}

UPhysicalMaterial*
UGroundSurfaceSoundComponent::DetectGroundPhysicalMaterial() const
{
    AActor* Owner = GetOwner();

    if (!Owner || !GetWorld())
    {
        return nullptr;
    }

    const FVector Start =
        Owner->GetActorLocation();

    const FVector End =
        Start - FVector(
            0.0f,
            0.0f,
            TraceDistance);

    FHitResult HitResult;

    FCollisionQueryParams QueryParams;

    QueryParams.AddIgnoredActor(Owner);
    QueryParams.bReturnPhysicalMaterial = true;

    const bool bHit =
        GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            TraceChannel,
            QueryParams);

    if (!bHit)
    {
        return nullptr;
    }

    return HitResult.PhysMaterial.Get();
}

USoundBase*
UGroundSurfaceSoundComponent::FindSoundForPhysicalMaterial(
    UPhysicalMaterial* PhysicalMaterial) const
{
    if (!PhysicalMaterial)
    {
        return nullptr;
    }

    for (const FGroundSurfaceSoundEntry& Entry : SurfaceSounds)
    {
        if (Entry.PhysicalMaterial == PhysicalMaterial)
        {
            return Entry.Sound;
        }
    }

    return nullptr;
}

float UGroundSurfaceSoundComponent::CalculateSpeedVolume(
    float Speed) const
{
    if (MaxSpeed <= 0.0f ||
        MaxVolume <= 0.0f)
    {
        return 0.0f;
    }

    /*
     * Normalize the vehicle speed to a 0.0 - 1.0 range.
     * MaxSpeed determines the speed at which maximum volume
     * is reached.
     */
    const float NormalizedSpeed =
        FMath::Clamp(
            FMath::Abs(Speed) / MaxSpeed,
            0.0f,
            1.0f);

    /*
     * Apply the normalized speed directly to the volume.
     * This creates a linear relationship between speed
     * and volume.
     */
    return NormalizedSpeed * MaxVolume;
}

void UGroundSurfaceSoundComponent::StartCrossfade(
    USoundBase* NewSound)
{
    if (!NewSound)
    {
        return;
    }

    /*
     * If another crossfade is already running,
     * resolve it first and use the currently fading-in
     * component as the active component.
     */
    if (bIsCrossfading)
    {
        if (CrossfadeOldComponent &&
            CrossfadeNewComponent)
        {
            const float Duration =
                FMath::Max(
                    CrossfadeDuration,
                    0.01f);

            const float Alpha =
                FMath::Clamp(
                    CrossfadeTime / Duration,
                    0.0f,
                    1.0f);

            CrossfadeOldComponent->SetVolumeMultiplier(
                FMath::Lerp(
                    CrossfadeStartOldVolume,
                    0.0f,
                    Alpha));

            CrossfadeNewComponent->SetVolumeMultiplier(
                FMath::Lerp(
                    0.0f,
                    CrossfadeTargetNewVolume,
                    Alpha));

            CrossfadeOldComponent->Stop();

            CrossfadeOldComponent->SetVolumeMultiplier(
                0.0f);

            bAudioAIsActive =
                CrossfadeNewComponent == AudioComponentA;

            CrossfadeNewComponent->SetVolumeMultiplier(
                CurrentGroundVolume);
        }

        CrossfadeOldComponent = nullptr;
        CrossfadeNewComponent = nullptr;

        bIsCrossfading = false;
        CrossfadeTime = 0.0f;
    }

    UAudioComponent* OldComponent =
        GetActiveAudioComponent();

    UAudioComponent* NewComponent =
        GetInactiveAudioComponent();

    if (!OldComponent || !NewComponent)
    {
        return;
    }

    /*
     * Prepare the new sound.
     */
    NewComponent->Stop();

    NewComponent->SetSound(NewSound);

    NewComponent->SetVolumeMultiplier(0.0f);

    /*
     * Start it silently and fade it in.
     */
    if (CurrentGroundVolume > 0.0f)
    {
        NewComponent->Play();
    }

    CrossfadeTime = 0.0f;

    CrossfadeStartOldVolume =
        OldComponent->IsPlaying()
        ? OldComponent->VolumeMultiplier
        : 0.0f;

    CrossfadeTargetNewVolume =
        CurrentGroundVolume;

    CrossfadeOldComponent =
        OldComponent;

    CrossfadeNewComponent =
        NewComponent;

    bIsCrossfading = true;

    /*
     * New Component becomes the active component.
     */
    bAudioAIsActive =
        !bAudioAIsActive;

    CurrentSound =
        NewSound;

    /*
     * Nothing to crossfade when both volumes are zero.
     */
    if (CrossfadeStartOldVolume <= 0.0f &&
        CrossfadeTargetNewVolume <= 0.0f)
    {
        OldComponent->Stop();
        NewComponent->Stop();

        NewComponent->SetVolumeMultiplier(0.0f);

        CrossfadeOldComponent = nullptr;
        CrossfadeNewComponent = nullptr;

        bIsCrossfading = false;
    }
}

void UGroundSurfaceSoundComponent::UpdateCrossfade(
    float DeltaTime)
{
    if (!bIsCrossfading)
    {
        return;
    }

    if (!CrossfadeOldComponent ||
        !CrossfadeNewComponent)
    {
        bIsCrossfading = false;

        CrossfadeOldComponent = nullptr;
        CrossfadeNewComponent = nullptr;

        return;
    }

    CrossfadeTime +=
        FMath::Max(DeltaTime, 0.0f);

    const float Duration =
        FMath::Max(
            CrossfadeDuration,
            0.01f);

    const float Alpha =
        FMath::Clamp(
            CrossfadeTime / Duration,
            0.0f,
            1.0f);

    /*
     * Fade the old sound out.
     */
    CrossfadeOldComponent->SetVolumeMultiplier(
        FMath::Lerp(
            CrossfadeStartOldVolume,
            0.0f,
            Alpha));

    /*
     * Fade the new sound in.
     */
    CrossfadeNewComponent->SetVolumeMultiplier(
        FMath::Lerp(
            0.0f,
            CrossfadeTargetNewVolume,
            Alpha));

    if (Alpha >= 1.0f)
    {
        CrossfadeOldComponent->Stop();

        CrossfadeOldComponent->SetVolumeMultiplier(
            0.0f);

        /*
         * Apply the current speed volume after the fade.
         */
        CrossfadeNewComponent->SetVolumeMultiplier(
            CurrentGroundVolume);

        /*
         * If the vehicle was stopped during the crossfade,
         * keep the component stopped.
         */
        if (CurrentGroundVolume <= 0.0f)
        {
            CrossfadeNewComponent->Stop();
        }
        else if (!CrossfadeNewComponent->IsPlaying())
        {
            CrossfadeNewComponent->Play();
        }

        CrossfadeOldComponent = nullptr;
        CrossfadeNewComponent = nullptr;

        bIsCrossfading = false;
        CrossfadeTime = 0.0f;
    }
}

UAudioComponent*
UGroundSurfaceSoundComponent::GetActiveAudioComponent() const
{
    if (bAudioAIsActive)
    {
        return AudioComponentA;
    }

    return AudioComponentB;
}

UAudioComponent*
UGroundSurfaceSoundComponent::GetInactiveAudioComponent() const
{
    if (bAudioAIsActive)
    {
        return AudioComponentB;
    }

    return AudioComponentA;
}

UPhysicalMaterial*
UGroundSurfaceSoundComponent::GetCurrentPhysicalMaterial() const
{
    return CurrentPhysicalMaterial;
}

USoundBase*
UGroundSurfaceSoundComponent::GetCurrentSound() const
{
    return CurrentSound;
}

float UGroundSurfaceSoundComponent::GetGroundVolume() const
{
    return CurrentGroundVolume;
}
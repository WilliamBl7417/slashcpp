#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "slashcpp/Public/AttributeComponent/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Items/Sword.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"


ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	Attribute = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes Component"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}
bool ABaseCharacter::IsAlive() const
{
	return false;
}
void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)//
	{
		DirectionalHitReaction(Hitter->GetActorLocation());
	}
	else
	{
		Die();
	}
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}
void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}
void ABaseCharacter::DodgeEnd()
{
}
void ABaseCharacter::Die()
{
	Tags.Add(FName("Dead"));
	PlayDeathmontage();
}
int32 ABaseCharacter::PlayAttackmontage()
{
	return  PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}
int32 ABaseCharacter::PlayDeathmontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	//si no hay animaciones
	if (Selection == -1)
	{
		StartRagDoll();
		return -1;
	}

	//si hay animaciones
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}

	return Selection;
}
void ABaseCharacter::PlayDogeMontage()
{
	playMontageSection(DodgeMontage, FName("Default"));
}
void ABaseCharacter::stopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}
FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;

	return CombatTargetLocation + TargetToMe;
}
FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}
void ABaseCharacter::StartRagDoll()
{
	{
		USkeletalMeshComponent* MeshComp = GetMesh();
		if (MeshComp)
		{
			MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
			MeshComp->SetSimulatePhysics(true);
			MeshComp->SetAllBodiesSimulatePhysics(true);
			MeshComp->WakeAllRigidBodies();
		}
		// Opcional: Detener animaciones
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.2f);
		}
	}

}
void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
bool ABaseCharacter::canAttack()
{
	return false;
}
bool ABaseCharacter::IsAlive()
{
	return true;
}
void ABaseCharacter::AttackEnd()
{
}
void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReacMontage)
	{
		AnimInstance->Montage_Play(HitReacMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReacMontage);
	}
}
void ABaseCharacter::DirectionalHitReaction(const FVector& ImpactPoint)
{
	/*
	* todo este codigo es para dibujar la direccion del golpe(deberia intentar entenderlo porque es importante y util)
	*/
	const FVector Forward = GetActorForwardVector();
	//Lower impact point to the enemy actor location z
	const FVector ImpactLower(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLower - GetActorLocation()).GetSafeNormal();

	//Forward * tohit = |forward||ToHit| * cost(theta)
	//|Forward| = 1 |tohit| = 1, so forward * tohit = cost(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);

	//Take the inverse cosin(arc-cosine) of cos(theta) to get theta
	double Theta = FMath::Acos(CosTheta);

	//convert to gradiance to degrees
	Theta = FMath::RadiansToDegrees(Theta);
	//if cross product points down, theta sohould be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);

	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("ReactBack");

	if (Theta >= -60.f && Theta <= 60.f)
	{
		Section = FName("ReactFront");
	}
	else if (Theta < -60.f && Theta >= -120.f)
	{
		Section = FName("ReactLeft");
	}
	else if (Theta > 60.f && Theta <= 120.f)
	{
		Section = FName("ReactRight");
	}

	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + CrossProduct * 100.f, 5.f, FColor::Blue, 5.f);

	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("theta %f"), Theta));
	}

	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward * 60.f, 5.f, FColor::Red, 5.f);

	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit * 60.f, 5.f, FColor::Green, 5.f);

	PlayHitReactMontage(Section);
}
void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
	}
}
void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		const FVector SpawnLocation = ImpactPoint;
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticles,
			SpawnLocation,
			FRotator::ZeroRotator,
			true
		);
	}
}
void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attribute)
	{
		Attribute->ReceiveDamage(DamageAmount);	

		if (Attribute->IsDead())
		{
			Die();
		}

/*		if (Attribute && Attribute->GetHealth() <= 0.f)
		{
			Die();
		}
		
*/
	}
}
void ABaseCharacter::playMontageSection(UAnimMontage* Montage, const FName& SectionName)
{  
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}
int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0 || !Montage) return -1; // Retorno seguro si no hay secciones

	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	playMontageSection(Montage, SectionNames[Selection]);

	return Selection;
}
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}
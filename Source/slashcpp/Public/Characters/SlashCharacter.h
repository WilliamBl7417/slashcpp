// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "CharacterTypes.h"
#include "Interfaces/PickUpInterface.h"
#include "SlashCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class ASoul;
class ATreasure;
class UAnimMontage;
class ASword;
class UCombatComponent;
class USlashOverlay;

UCLASS()
class SLASHCPP_API ASlashCharacter : public ABaseCharacter, public IPickUpInterface
{
	GENERATED_BODY()

public:

	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual float TakeDamage(float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void ChangeState(EActionState NewState);

	//interfaces//estas no se pueden editar en blueprints para eso se hacecon
	//_inplementation// puede investigar sobre eso se hace un poco diferente
	virtual void SetOverlapingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* soul) override;
	virtual void AddGold(ATreasure* Treasure) override;

	//this is just trash line for test git hub pull and push
	//this is just trash line for the second time for test git hub pull and push


protected:
	virtual void BeginPlay() override;

	virtual bool IsAlive() const override;

	/***CallBacks Inputs*/
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EKeyPressed();
	void EquipWeapon(ASword* Weapon);
	virtual void Attack() override;
	void Dodge();

	bool HasEnoughStamina();

	bool IsUnoccupied();

	/**Combat related Functions*/
	virtual void AttackEnd() override;
	virtual bool canAttack() override;

	virtual void DodgeEnd() override;

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();
	void PlayEquipMontage(FName SectionName);
	bool HaveWeapon();
	bool canDisarm();
	bool canArm();
	void disarm();
	void arm();
	virtual void Die() override;

private:

	//bool IsUnoccupied();
	void InitializeSlashOverlay();
	void SetHUDHealth();

	/**Character Components*/
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArmComponent;
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* CameraComponent;
	UPROPERTY(VisibleAnywhere)
	UGroomComponent* HairMesh;
	UPROPERTY(VisibleAnywhere)
	UGroomComponent* EyeBrowsMesh;

	/**Character Related Variables*/
	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlapingItem;
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EquipMontage;

	/** ATENCION!!!!!  */
	/**esto podria dar problemas por ponerlo aqui */
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

public:

	
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};

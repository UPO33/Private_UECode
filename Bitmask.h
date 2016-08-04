UENUM(Blueprintable, meta=(Bitflags))
enum ELayer
{
	EL_Layer0,
	EL_Layer1,
	EL_Layer2,
	EL_Layer3,
	EL_Layer4
};

UPROPERTY(EditAnywhere, meta=(Bitmask, BitmaskEnum="ELayer"))
int32   _Layer;
UPROPERTY(EditAnywhere, meta=(Bitmask, BitmaskEnum="ECollisionChannel"))
int32	ObjectTypes;
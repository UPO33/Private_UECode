
	/*
	easy timer for unreal , add the following codes to an actor class
	*/
private:
	static const unsigned MAX_TIMER = 32;
	FTimerHandle mTimerHandles[MAX_TIMER];

	FTimerHandle* GetFreerTimerHandle()
	{
		for (unsigned i = 0; i < MAX_TIMER; i++)
		{
			if (!mTimerHandles[i].IsValid()) return mTimerHandles + i;
		}
		return nullptr;
	}
public:
	/*
	example:
		DoAfter(1, [this](){
			AddActorWorldOffset(FVector(10,0,0);
			DoAfter(0.5, [this](){
				AddActorWorldOffset(FVector(0,10,0);
			});
			DoAfter(2.5, [this](){
				Destroy();
			});
		});
	*/
	template<typename Lambda> void DoAfter(float seconds, Lambda proc)
	{
		FTimerHandle* handle = GetFreerTimerHandle();
		check(handle);

		FTimerManager* timer = &GetWorld()->GetTimerManager();
		timer->SetTimer(*handle, 
			[proc, handle, timer]()
			{
				proc();
				timer->ClearTimer(*handle);
			}
		, seconds, false);
	}

	void Destroyed() override
	{
		Super::Destroyed();
		for (FTimerHandle& handle : mTimerHandles)
			GetWorld()->GetTimerManager().ClearTimer(handle);
	}

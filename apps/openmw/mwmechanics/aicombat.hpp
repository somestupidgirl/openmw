#ifndef GAME_MWMECHANICS_AICOMBAT_H
#define GAME_MWMECHANICS_AICOMBAT_H

#include "aitemporarybase.hpp"
#include "aitimer.hpp"
#include "movement.hpp"
#include "typedaipackage.hpp"

#include "../mwworld/cellstore.hpp" // for Doors

#include <components/esm3/loadpgrd.hpp>

namespace ESM
{
    namespace AiSequence
    {
        struct AiCombat;
    }
}

namespace MWMechanics
{
    class Action;

    /// \brief This class holds the variables AiCombat needs which are deleted if the package becomes inactive.
    struct AiCombatStorage : AiTemporaryBase
    {
        float mAttackCooldown;
        AiReactionTimer mReaction;
        float mTimerCombatMove;
        bool mReadyToAttack;
        bool mAttack;
        float mAttackRange;
        bool mCombatMove;
        bool mRotateMove;
        osg::Vec3f mLastTargetPos;
        const MWWorld::CellStore* mCell;
        std::unique_ptr<Action> mCurrentAction;
        float mActionCooldown;
        float mStrength;
        bool mForceNoShortcut;
        ESM::Position mShortcutFailPos;
        MWMechanics::Movement mMovement;

        enum FleeState
        {
            FleeState_None,
            FleeState_Idle,
            FleeState_RunBlindly,
            FleeState_RunToDestination
        };
        FleeState mFleeState;
        bool mLOS;
        float mUpdateLOSTimer;
        float mFleeBlindRunTimer;
        ESM::Pathgrid::Point mFleeDest;

        bool mUseCustomDestination;
        osg::Vec3f mCustomDestination;

        AiCombatStorage();

        void startCombatMove(bool isDistantCombat, float distToTarget, float rangeAttack, const MWWorld::Ptr& actor,
            const MWWorld::Ptr& target);
        void updateCombatMove(float duration);
        void stopCombatMove();
        void startAttackIfReady(const MWWorld::Ptr& actor, CharacterController& characterController,
            const ESM::Weapon* weapon, bool distantCombat, bool canShout);
        void updateAttack(const MWWorld::Ptr& actor, CharacterController& characterController);
        void stopAttack();

        void startFleeing();
        void stopFleeing();
        bool isFleeing() const;
    };

    /// \brief Causes the actor to fight another actor
    class AiCombat final : public TypedAiPackage<AiCombat>
    {
    public:
        /// Constructor
        /** \param actor Actor to fight **/
        explicit AiCombat(const MWWorld::Ptr& actor);

        explicit AiCombat(const ESM::AiSequence::AiCombat* combat);

        void init();

        bool execute(const MWWorld::Ptr& actor, CharacterController& characterController, AiState& state,
            float duration) override;

        static constexpr AiPackageTypeId getTypeId() { return AiPackageTypeId::Combat; }

        static constexpr Options makeDefaultOptions()
        {
            AiPackage::Options options;
            options.mPriority = 1;
            options.mCanCancel = false;
            options.mShouldCancelPreviousAi = false;
            return options;
        }

        /// Returns target ID
        MWWorld::Ptr getTarget() const override;

        void writeState(ESM::AiSequence::AiSequence& sequence) const override;

    private:
        /// Returns true if combat should end
        bool attack(const MWWorld::Ptr& actor, const MWWorld::Ptr& target, AiCombatStorage& storage,
            CharacterController& characterController);

        void updateLOS(const MWWorld::Ptr& actor, const MWWorld::Ptr& target, float duration, AiCombatStorage& storage);

        void updateFleeing(const MWWorld::Ptr& actor, const MWWorld::Ptr& target, float duration,
            MWWorld::MovementDirectionFlags supportedMovementDirections, AiCombatStorage& storage);

        /// Transfer desired movement (from AiCombatStorage) to Actor
        void updateActorsMovement(const MWWorld::Ptr& actor, float duration, AiCombatStorage& storage);
        void rotateActorOnAxis(const MWWorld::Ptr& actor, int axis, MWMechanics::Movement& actorMovementSettings,
            AiCombatStorage& storage);
    };

}

#endif

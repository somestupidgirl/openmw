#ifndef GAME_MWBASE_MECHANICSMANAGER_H
#define GAME_MWBASE_MECHANICSMANAGER_H

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "../mwworld/ptr.hpp"

namespace osg
{
    class Stats;
    class Vec3f;
}

namespace ESM
{
    struct Class;
    class RefId;
    class ESMReader;
    class ESMWriter;
}

namespace MWMechanics
{
    enum class GreetingState;
}

namespace MWWorld
{
    class Ptr;
    class CellStore;
    class CellRef;
}

namespace Loading
{
    class Listener;
}

namespace MWBase
{
    /// \brief Interface for game mechanics manager (implemented in MWMechanics)
    class MechanicsManager
    {
        MechanicsManager(const MechanicsManager&);
        ///< not implemented

        MechanicsManager& operator=(const MechanicsManager&);
        ///< not implemented

    public:
        MechanicsManager() {}

        virtual ~MechanicsManager() {}

        virtual void add(const MWWorld::Ptr& ptr) = 0;
        ///< Register an object for management

        virtual void remove(const MWWorld::Ptr& ptr, bool keepActive) = 0;
        ///< Deregister an object for management

        virtual void updateCell(const MWWorld::Ptr& old, const MWWorld::Ptr& ptr) = 0;
        ///< Moves an object to a new cell

        virtual void drop(const MWWorld::CellStore* cellStore) = 0;
        ///< Deregister all objects in the given cell.

        virtual void setPlayerName(const std::string& name) = 0;
        ///< Set player name.

        virtual void setPlayerRace(const ESM::RefId& id, bool male, const ESM::RefId& head, const ESM::RefId& hair) = 0;
        ///< Set player race.

        virtual void setPlayerBirthsign(const ESM::RefId& id) = 0;
        ///< Set player birthsign.

        virtual void setPlayerClass(const ESM::RefId& id) = 0;
        ///< Set player class to stock class.

        virtual void setPlayerClass(const ESM::Class& class_) = 0;
        ///< Set player class to custom class.

        virtual void restoreDynamicStats(const MWWorld::Ptr& actor, double hours, bool sleep) = 0;

        virtual void rest(double hours, bool sleep) = 0;
        ///< If the player is sleeping or waiting, this should be called every hour.
        /// @param sleep is the player sleeping or waiting?

        virtual int getHoursToRest() const = 0;
        ///< Calculate how many hours the player needs to rest in order to be fully healed

        virtual int getBarterOffer(const MWWorld::Ptr& ptr, int basePrice, bool buying) = 0;
        ///< This is used by every service to determine the price of objects given the trading skills of the player and
        ///< NPC.

        virtual int getDerivedDisposition(const MWWorld::Ptr& ptr, bool clamp = true) = 0;
        ///< Calculate the diposition of an NPC toward the player.

        virtual int countDeaths(const ESM::RefId& id) const = 0;
        ///< Return the number of deaths for actors with the given ID.

        /// Check if \a observer is potentially aware of \a ptr. Does not do a line of sight check!
        virtual bool awarenessCheck(const MWWorld::Ptr& ptr, const MWWorld::Ptr& observer) = 0;

        /// Makes \a ptr fight \a target. Also shouts a combat taunt.
        virtual void startCombat(
            const MWWorld::Ptr& ptr, const MWWorld::Ptr& target, const std::set<MWWorld::Ptr>* targetAllies)
            = 0;

        /// Removes an actor and its allies from combat with the actor's targets.
        virtual void stopCombat(const MWWorld::Ptr& ptr) = 0;

        enum OffenseType
        {
            OT_Theft, // Taking items owned by an NPC or a faction you are not a member of
            OT_Assault, // Attacking a peaceful NPC
            OT_Murder, // Murdering a peaceful NPC
            OT_Trespassing, // Picking the lock of an owned door/chest
            OT_SleepingInOwnedBed, // Sleeping in a bed owned by an NPC or a faction you are not a member of
            OT_Pickpocket // Entering pickpocket mode, leaving it, and being detected. Any items stolen are a separate
                          // crime (Theft)
        };
        /**
         * @note victim may be empty
         * @param arg Depends on \a type, e.g. for Theft, the value of the item that was stolen.
         * @param victimAware Is the victim already aware of the crime?
         *                    If this parameter is false, it will be determined by a line-of-sight and awareness check.
         * @return was the crime seen?
         */
        virtual bool commitCrime(const MWWorld::Ptr& ptr, const MWWorld::Ptr& victim, OffenseType type,
            const ESM::RefId& factionId = ESM::RefId(), int arg = 0, bool victimAware = false)
            = 0;
        /// @return false if the attack was considered a "friendly hit" and forgiven
        virtual bool actorAttacked(const MWWorld::Ptr& victim, const MWWorld::Ptr& attacker) = 0;

        /// Notify that actor was killed, add a murder bounty if applicable
        /// @note No-op for non-player attackers
        virtual void actorKilled(const MWWorld::Ptr& victim, const MWWorld::Ptr& attacker) = 0;

        /// Utility to check if taking this item is illegal and calling commitCrime if so
        /// @param container The container the item is in; may be empty for an item in the world
        virtual void itemTaken(const MWWorld::Ptr& ptr, const MWWorld::Ptr& item, const MWWorld::Ptr& container,
            int count, bool alarm = true)
            = 0;
        /// Utility to check if unlocking this object is illegal and calling commitCrime if so
        virtual void unlockAttempted(const MWWorld::Ptr& ptr, const MWWorld::Ptr& item) = 0;
        /// Attempt sleeping in a bed. If this is illegal, call commitCrime.
        /// @return was it illegal, and someone saw you doing it?
        virtual bool sleepInBed(const MWWorld::Ptr& ptr, const MWWorld::Ptr& bed) = 0;

        enum PersuasionType
        {
            PT_Admire,
            PT_Intimidate,
            PT_Taunt,
            PT_Bribe10,
            PT_Bribe100,
            PT_Bribe1000
        };
        virtual void getPersuasionDispositionChange(
            const MWWorld::Ptr& npc, PersuasionType type, bool& success, int& tempChange, int& permChange)
            = 0;
        ///< Perform a persuasion action on NPC

        virtual void forceStateUpdate(const MWWorld::Ptr& ptr) = 0;
        ///< Forces an object to refresh its animation state.

        virtual bool playAnimationGroup(
            const MWWorld::Ptr& ptr, std::string_view groupName, int mode, uint32_t number = 1, bool scripted = false)
            = 0;
        ///< Run animation for a MW-reference. Calls to this function for references that are currently not
        /// in the scene should be ignored.
        ///
        /// \param mode 0 normal, 1 immediate start, 2 immediate loop
        /// \param number How many times the animation should be run
        /// \param scripted Whether the animation should be treated as a scripted animation.
        /// \return Success or error
        virtual bool playAnimationGroupLua(const MWWorld::Ptr& ptr, std::string_view groupName, uint32_t loops,
            float speed, std::string_view startKey, std::string_view stopKey, bool forceLoop)
            = 0;
        ///< Lua variant of playAnimationGroup. The mode parameter is omitted
        /// and forced to 0. modes 1 and 2 can be emulated by doing clearAnimationQueue() and
        /// setting the startKey.
        ///
        /// \param number How many times the animation should be run
        /// \param speed How fast to play the animation, where 1.f = normal speed
        /// \param startKey Which textkey to start the animation from
        /// \param stopKey Which textkey to stop the animation on
        /// \param forceLoop Force the animation to be looping, even if it's normally not looping.
        /// \param blendMask See MWRender::Animation::BlendMask
        /// \param scripted Whether the animation should be treated as as scripted animation
        /// \return Success or error
        ///

        virtual void enableLuaAnimations(const MWWorld::Ptr& ptr, bool enable) = 0;

        virtual void skipAnimation(const MWWorld::Ptr& ptr) = 0;
        ///< Skip the animation for the given MW-reference for one frame. Calls to this function for
        /// references that are currently not in the scene should be ignored.

        virtual bool checkAnimationPlaying(const MWWorld::Ptr& ptr, std::string_view groupName) = 0;

        virtual bool checkScriptedAnimationPlaying(const MWWorld::Ptr& ptr) const = 0;

        /// Save the current animation state of managed references to their RefData.
        virtual void persistAnimationStates() = 0;

        /// Clear out the animation queue, and cancel any animation currently playing from the queue
        virtual void clearAnimationQueue(const MWWorld::Ptr& ptr, bool clearScripted) = 0;

        /// Update magic effects for an actor. Usually done automatically once per frame, but if we're currently
        /// paused we may want to do it manually (after equipping permanent enchantment)
        virtual void updateMagicEffects(const MWWorld::Ptr& ptr) = 0;

        virtual bool toggleAI() = 0;
        virtual bool isAIActive() = 0;

        virtual void getObjectsInRange(const osg::Vec3f& position, float radius, std::vector<MWWorld::Ptr>& objects)
            = 0;
        virtual void getActorsInRange(const osg::Vec3f& position, float radius, std::vector<MWWorld::Ptr>& objects) = 0;

        /// Check if there are actors in selected range
        virtual bool isAnyActorInRange(const osg::Vec3f& position, float radius) = 0;

        /// Returns the list of actors which are siding with the given actor in fights
        /**ie AiFollow or AiEscort is active and the target is the actor **/
        virtual std::vector<MWWorld::Ptr> getActorsSidingWith(const MWWorld::Ptr& actor) = 0;
        virtual std::vector<MWWorld::Ptr> getActorsFollowing(const MWWorld::Ptr& actor) = 0;
        virtual std::vector<int> getActorsFollowingIndices(const MWWorld::Ptr& actor) = 0;
        virtual std::map<int, MWWorld::Ptr> getActorsFollowingByIndex(const MWWorld::Ptr& actor) = 0;

        /// Returns a list of actors who are fighting the given actor within the fAlarmDistance
        /** ie AiCombat is active and the target is the actor **/
        virtual std::vector<MWWorld::Ptr> getActorsFighting(const MWWorld::Ptr& actor) = 0;

        virtual std::vector<MWWorld::Ptr> getEnemiesNearby(const MWWorld::Ptr& actor) = 0;

        /// Recursive versions of above methods
        virtual void getActorsFollowing(const MWWorld::Ptr& actor, std::set<MWWorld::Ptr>& out) = 0;
        virtual void getActorsSidingWith(const MWWorld::Ptr& actor, std::set<MWWorld::Ptr>& out) = 0;

        virtual void playerLoaded() = 0;

        virtual int countSavedGameRecords() const = 0;

        virtual void write(ESM::ESMWriter& writer, Loading::Listener& listener) const = 0;

        virtual void readRecord(ESM::ESMReader& reader, uint32_t type) = 0;

        virtual void clear() = 0;

        virtual bool isAggressive(const MWWorld::Ptr& ptr, const MWWorld::Ptr& target) = 0;

        /// Resurrects the player if necessary
        virtual void resurrect(const MWWorld::Ptr& ptr) = 0;

        virtual bool isCastingSpell(const MWWorld::Ptr& ptr) const = 0;
        virtual bool isReadyToBlock(const MWWorld::Ptr& ptr) const = 0;
        virtual bool isAttackingOrSpell(const MWWorld::Ptr& ptr) const = 0;

        virtual void castSpell(const MWWorld::Ptr& ptr, const ESM::RefId& spellId, bool scriptedSpell) = 0;

        virtual void processChangedSettings(const std::set<std::pair<std::string, std::string>>& settings) = 0;

        virtual void notifyDied(const MWWorld::Ptr& actor) = 0;

        virtual bool onOpen(const MWWorld::Ptr& ptr) = 0;
        virtual void onClose(const MWWorld::Ptr& ptr) = 0;

        /// Check if the target actor was detected by an observer
        /// If the observer is a non-NPC, check all actors in AI processing distance as observers
        virtual bool isActorDetected(const MWWorld::Ptr& actor, const MWWorld::Ptr& observer) = 0;

        virtual void confiscateStolenItems(const MWWorld::Ptr& player, const MWWorld::Ptr& targetContainer) = 0;

        /// List the owners that the player has stolen this item from (the owner can be an NPC or a faction).
        /// <Owner, item count>
        virtual std::vector<std::pair<ESM::RefId, int>> getStolenItemOwners(const ESM::RefId& itemid) = 0;

        /// Has the player stolen this item from the given owner?
        virtual bool isItemStolenFrom(const ESM::RefId& itemid, const MWWorld::Ptr& ptr) = 0;

        virtual bool isBoundItem(const MWWorld::Ptr& item) = 0;
        virtual bool isAllowedToUse(const MWWorld::Ptr& ptr, const MWWorld::Ptr& target, MWWorld::Ptr& victim) = 0;

        /// Turn actor into werewolf or normal form.
        virtual void setWerewolf(const MWWorld::Ptr& actor, bool werewolf) = 0;

        /// Sets the NPC's Acrobatics skill to match the fWerewolfAcrobatics GMST.
        /// It only applies to the current form the NPC is in.
        virtual void applyWerewolfAcrobatics(const MWWorld::Ptr& actor) = 0;

        virtual void cleanupSummonedCreature(const MWWorld::Ptr& caster, int creatureActorId) = 0;

        virtual void confiscateStolenItemToOwner(
            const MWWorld::Ptr& player, const MWWorld::Ptr& item, const MWWorld::Ptr& victim, int count)
            = 0;
        virtual bool isAttackPreparing(const MWWorld::Ptr& ptr) = 0;
        virtual bool isRunning(const MWWorld::Ptr& ptr) = 0;
        virtual bool isSneaking(const MWWorld::Ptr& ptr) = 0;

        virtual void reportStats(unsigned int frameNumber, osg::Stats& stats) const = 0;

        virtual int getGreetingTimer(const MWWorld::Ptr& ptr) const = 0;
        virtual float getAngleToPlayer(const MWWorld::Ptr& ptr) const = 0;
        virtual MWMechanics::GreetingState getGreetingState(const MWWorld::Ptr& ptr) const = 0;
        virtual bool isTurningToPlayer(const MWWorld::Ptr& ptr) const = 0;
    };
}

#endif

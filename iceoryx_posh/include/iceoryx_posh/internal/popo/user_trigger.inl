// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef IOX_POSH_POPO_USER_TRIGGER_INL
#define IOX_POSH_POPO_USER_TRIGGER_INL
namespace iox
{
namespace popo
{
template <uint64_t WaitSetCapacity>
inline cxx::expected<WaitSetError> UserTrigger::attachTo(WaitSet<WaitSetCapacity>& waitset,
                                                         const uint64_t triggerId,
                                                         const Trigger::Callback<UserTrigger> callback) noexcept
{
    return waitset
        .acquireTrigger(
            this, {*this, &UserTrigger::hasTriggered}, {*this, &UserTrigger::invalidateTrigger}, triggerId, callback)
        .and_then([this](TriggerHandle& trigger) { m_trigger = std::move(trigger); });
}

template <uint64_t WaitSetCapacity>
inline cxx::expected<WaitSetError> UserTrigger::attachTo(WaitSet<WaitSetCapacity>& waitset,
                                                         const Trigger::Callback<UserTrigger> callback) noexcept
{
    return attachTo(waitset, Trigger::INVALID_TRIGGER_ID, callback);
}
} // namespace popo
} // namespace iox
#endif

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

#include "iceoryx_posh/popo/modern_api/untyped_subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

iox::popo::UserTrigger shutdownTrigger;

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownTrigger.trigger();
}

// The callback of the trigger. Every callback must have an argument which is
// a pointer to the origin of the Trigger. In our case the trigger origin is
// the untyped subscriber.
void subscriberCallback(iox::popo::UntypedSubscriber* const subscriber)
{
    subscriber->take().and_then([&](iox::popo::Sample<const void>& sample) {
        std::cout << "subscriber: " << std::hex << subscriber << " length: " << std::dec
                  << sample.getHeader()->m_info.m_payloadSize << " ptr: " << std::hex << sample.getHeader()->payload()
                  << std::endl;
    });
}

int main()
{
    constexpr uint64_t NUMBER_OF_SUBSCRIBERS = 4U;

    signal(SIGINT, sigHandler);

    iox::runtime::PoshRuntime::initRuntime("/iox-ex-waitset-gateway");

    iox::popo::WaitSet<NUMBER_OF_SUBSCRIBERS + 1> waitset;

    // attach shutdownTrigger to handle CTRL+C
    shutdownTrigger.attachTo(waitset);


    // create subscriber and subscribe them to our service
    iox::cxx::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
    for (auto i = 0; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
        auto& subscriber = subscriberVector.back();

        subscriber.subscribe();
        subscriber.attachTo(waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES, subscriberCallback);
    }

    // event loop
    while (true)
    {
        auto triggerVector = waitset.wait();

        for (auto& trigger : triggerVector)
        {
            if (trigger.doesOriginateFrom(&shutdownTrigger))
            {
                // CTRL+c was pressed -> exit
                return (EXIT_SUCCESS);
            }
            else
            {
                // call the callback which was assigned to the trigger
                trigger();
            }
        }

        std::cout << std::endl;
    }

    return (EXIT_SUCCESS);
}

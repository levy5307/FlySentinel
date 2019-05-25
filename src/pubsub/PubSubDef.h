//
// Created by levy on 2019/5/18.
//

#ifndef FLYSENTINEL_PUBSUBDEF_H
#define FLYSENTINEL_PUBSUBDEF_H

#include "../coordinator/interface/AbstractFlyClient.h"

struct PubSubPattern {
    PubSubPattern(std::shared_ptr<AbstractFlyClient> flyClient, const std::string& pattern)
            : flyClient(flyClient), pattern(pattern) {

    }
    std::shared_ptr<AbstractFlyClient> flyClient;
    const std::string& pattern;
};

#endif //FLYSENTINEL_PUBSUBDEF_H

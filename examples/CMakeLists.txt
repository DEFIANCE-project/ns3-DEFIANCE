build_lib_example(
        NAME defiance-addition-example
        SOURCE_FILES
                addition-example/addition-agent-app.cc
                addition-example/no-op-action-app.cc
                addition-example/node-id-reward-app.cc
                addition-example/random-observation-app.cc
                addition-example/simulation.cc
        HEADER_FILES
                addition-example/addition-agent-app.h
                addition-example/no-op-action-app.h
                addition-example/node-id-reward-app.h
                addition-example/random-observation-app.h
        LIBRARIES_TO_LINK
                ${libdefiance}
)

build_lib_example(
        NAME defiance-agent-communication-example
        SOURCE_FILES agent-communication-example/agent-communication-example.cc
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${libmobility}
)

build_lib_example(
        NAME defiance-observation-sharing-example
        SOURCE_FILES
                observation-sharing-example/position-observation-app.cc
                observation-sharing-example/observation-sharing-agent-app.cc
                observation-sharing-example/simulation.cc
        HEADER_FILES
                observation-sharing-example/position-observation-app.h
                observation-sharing-example/observation-sharing-agent-app.h
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${libmobility}
)

build_lib_example(
        NAME defiance-app-communication-example
        SOURCE_FILES
                app-communication-example/simulation.cc
                app-communication-example/position-reward-app.cc
                app-communication-example/mobility-observation-app.cc
                app-communication-example/mobility-agent-app.cc
                app-communication-example/position-action-app.cc
        HEADER_FILES
                app-communication-example/position-observation-app.h
                app-communication-example/position-reward-app.h
                app-communication-example/position-agent-app.h
                app-communication-example/position-action-app.h
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${liblte}
                ${libapplications}
)

build_lib_example(
        NAME defiance-channel-interface-example
        SOURCE_FILES channel-interface-example/channel-interface-example.cc
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${liblte}
)

build_lib_example(
        NAME defiance-lte-test
        SOURCE_FILES lte-learning/test.cc
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${liblte}
)

build_lib_example(
        NAME defiance-pendulum
        SOURCE_FILES
                scenario/pendulum-cart/test-cart.cc
                scenario/pendulum-cart/pendulum-cart.cc
        HEADER_FILES
                scenario/pendulum-cart/pendulum-cart.h
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${liblte}
)

build_lib_example(
        NAME defiance-balance1
        SOURCE_FILES
                scenario/balance1-scenario.cc
                scenario/pendulum-cart/pendulum-cart.cc
                scenario/environment-creator.cc
        HEADER_FILES
                scenario/pendulum-cart/pendulum-cart.h
                scenario/environment-creator.h
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${libnetanim}
)

build_lib_example(
        NAME defiance-balance2
        SOURCE_FILES
                scenario/balance2-scenario.cc
                scenario/pendulum-cart/pendulum-cart.cc
                scenario/environment-creator.cc
        HEADER_FILES
                scenario/pendulum-cart/pendulum-cart.h
                scenario/environment-creator.h
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${libnetanim}
)

build_lib_example(
        NAME defiance-lte-animation
        SOURCE_FILES scenario/basic-lte-animation.cc
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${libnetanim}
)

build_lib_example(
        NAME defiance-environment-creator-example
        SOURCE_FILES
                scenario/environment-creator-example.cc
                scenario/environment-creator.cc
                scenario/pendulum-cart/pendulum-cart.cc
        HEADER_FILES
                scenario/environment-creator.h
                scenario/pendulum-cart/pendulum-cart.h
        LIBRARIES_TO_LINK
                ${libdefiance}
                ${liblte}
)

build_lib_example(
        NAME defiance-handover-example
        SOURCE_FILES
                handover-example/handover-action-application.cc
                handover-example/handover-agent-application.cc
                handover-example/handover-observation-application.cc
                handover-example/handover-reward-application.cc
                handover-example/simulation.cc
                handover-example/simulation-setup.cc
        HEADER_FILES
                handover-example/handover-action-application.h
                handover-example/handover-agent-application.h
                handover-example/handover-observation-application.h
                handover-example/handover-reward-application.h
        LIBRARIES_TO_LINK
                ${libapplications}
                ${liblte}
                ${libdefiance}
)

build_lib_example(
        NAME defiance-sumo-test
        SOURCE_FILES sumo-test/sumo-example.cc
        LIBRARIES_TO_LINK
                ${libapplications}
                ${libdefiance}
                ${libinternet}
                ${libmobility}
                ${libnetanim}
)

build_lib_example(
        NAME defiance-sumo-topology-creator
        SOURCE_FILES topology-creation/sumo-topology-creator.cc
        LIBRARIES_TO_LINK
                ${libapplications}
                ${libdefiance}
                ${libinternet}
                ${libmobility}
                ${libnetanim}
)
add_subdirectory(lte-learning)

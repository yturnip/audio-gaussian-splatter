//
// Created by Yohanes Turnip on 2026-07-08.
//

#include <juce_core/juce_core.h>

int main (int, char**)
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runAllTests();

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        auto* result = runner.getResult(i);
        if (result -> failures > 0)
            return 1;
    }
    return 0;
}
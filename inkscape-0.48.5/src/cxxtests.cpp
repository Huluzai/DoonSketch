/* Generated file, do not edit */

// -*- C++ -*-
//

#include "MultiPrinter.h"

int main( int argc, char *argv[] )
{
    (void)argc;
    return CxxTest::MultiPrinter( argv[0] ).run();
}

// The CxxTest "world"
#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#define _CXXTEST_HAVE_EH
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>

#include "../../src/attributes-test.h"

static AttributesTest *suite_AttributesTest = 0;

static CxxTest::List Tests_AttributesTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<AttributesTest> suiteDescription_AttributesTest( "../../src/attributes-test.h", 13, "AttributesTest", Tests_AttributesTest, suite_AttributesTest, 24, 25 );

static class TestDescription_AttributesTest_testAttributes : public CxxTest::RealTestDescription {
public:
 TestDescription_AttributesTest_testAttributes() : CxxTest::RealTestDescription( Tests_AttributesTest, suiteDescription_AttributesTest, 28, "testAttributes" ) {}
 void runTest() { if ( suite_AttributesTest ) suite_AttributesTest->testAttributes(); }
} testDescription_AttributesTest_testAttributes;

#include "../../src/color-profile-test.h"

static ColorProfileTest *suite_ColorProfileTest = 0;

static CxxTest::List Tests_ColorProfileTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<ColorProfileTest> suiteDescription_ColorProfileTest( "../../src/color-profile-test.h", 13, "ColorProfileTest", Tests_ColorProfileTest, suite_ColorProfileTest, 45, 51 );

static class TestDescription_ColorProfileTest_testSetRenderingIntent : public CxxTest::RealTestDescription {
public:
 TestDescription_ColorProfileTest_testSetRenderingIntent() : CxxTest::RealTestDescription( Tests_ColorProfileTest, suiteDescription_ColorProfileTest, 60, "testSetRenderingIntent" ) {}
 void runTest() { if ( suite_ColorProfileTest ) suite_ColorProfileTest->testSetRenderingIntent(); }
} testDescription_ColorProfileTest_testSetRenderingIntent;

static class TestDescription_ColorProfileTest_testSetLocal : public CxxTest::RealTestDescription {
public:
 TestDescription_ColorProfileTest_testSetLocal() : CxxTest::RealTestDescription( Tests_ColorProfileTest, suiteDescription_ColorProfileTest, 89, "testSetLocal" ) {}
 void runTest() { if ( suite_ColorProfileTest ) suite_ColorProfileTest->testSetLocal(); }
} testDescription_ColorProfileTest_testSetLocal;

static class TestDescription_ColorProfileTest_testSetName : public CxxTest::RealTestDescription {
public:
 TestDescription_ColorProfileTest_testSetName() : CxxTest::RealTestDescription( Tests_ColorProfileTest, suiteDescription_ColorProfileTest, 113, "testSetName" ) {}
 void runTest() { if ( suite_ColorProfileTest ) suite_ColorProfileTest->testSetName(); }
} testDescription_ColorProfileTest_testSetName;

#include "../../src/dir-util-test.h"

static DirUtilTest suite_DirUtilTest;

static CxxTest::List Tests_DirUtilTest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_DirUtilTest( "../../src/dir-util-test.h", 9, "DirUtilTest", suite_DirUtilTest, Tests_DirUtilTest );

static class TestDescription_DirUtilTest_testBase : public CxxTest::RealTestDescription {
public:
 TestDescription_DirUtilTest_testBase() : CxxTest::RealTestDescription( Tests_DirUtilTest, suiteDescription_DirUtilTest, 12, "testBase" ) {}
 void runTest() { suite_DirUtilTest.testBase(); }
} testDescription_DirUtilTest_testBase;

#include "../../src/extract-uri-test.h"

static ExtractURITest suite_ExtractURITest;

static CxxTest::List Tests_ExtractURITest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_ExtractURITest( "../../src/extract-uri-test.h", 9, "ExtractURITest", suite_ExtractURITest, Tests_ExtractURITest );

static class TestDescription_ExtractURITest_testBase : public CxxTest::RealTestDescription {
public:
 TestDescription_ExtractURITest_testBase() : CxxTest::RealTestDescription( Tests_ExtractURITest, suiteDescription_ExtractURITest, 26, "testBase" ) {}
 void runTest() { suite_ExtractURITest.testBase(); }
} testDescription_ExtractURITest_testBase;

static class TestDescription_ExtractURITest_testWithTrailing : public CxxTest::RealTestDescription {
public:
 TestDescription_ExtractURITest_testWithTrailing() : CxxTest::RealTestDescription( Tests_ExtractURITest, suiteDescription_ExtractURITest, 44, "testWithTrailing" ) {}
 void runTest() { suite_ExtractURITest.testWithTrailing(); }
} testDescription_ExtractURITest_testWithTrailing;

static class TestDescription_ExtractURITest_testQuoted : public CxxTest::RealTestDescription {
public:
 TestDescription_ExtractURITest_testQuoted() : CxxTest::RealTestDescription( Tests_ExtractURITest, suiteDescription_ExtractURITest, 58, "testQuoted" ) {}
 void runTest() { suite_ExtractURITest.testQuoted(); }
} testDescription_ExtractURITest_testQuoted;

#include "../../src/marker-test.h"

static MarkerTest suite_MarkerTest;

static CxxTest::List Tests_MarkerTest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_MarkerTest( "../../src/marker-test.h", 14, "MarkerTest", suite_MarkerTest, Tests_MarkerTest );

static class TestDescription_MarkerTest_testMarkerLoc : public CxxTest::RealTestDescription {
public:
 TestDescription_MarkerTest_testMarkerLoc() : CxxTest::RealTestDescription( Tests_MarkerTest, suiteDescription_MarkerTest, 18, "testMarkerLoc" ) {}
 void runTest() { suite_MarkerTest.testMarkerLoc(); }
} testDescription_MarkerTest_testMarkerLoc;

#include "../../src/mod360-test.h"

static Mod360Test suite_Mod360Test;

static CxxTest::List Tests_Mod360Test = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_Mod360Test( "../../src/mod360-test.h", 12, "Mod360Test", suite_Mod360Test, Tests_Mod360Test );

static class TestDescription_Mod360Test_testMod360 : public CxxTest::RealTestDescription {
public:
 TestDescription_Mod360Test_testMod360() : CxxTest::RealTestDescription( Tests_Mod360Test, suiteDescription_Mod360Test, 18, "testMod360" ) {}
 void runTest() { suite_Mod360Test.testMod360(); }
} testDescription_Mod360Test_testMod360;

#include "../../src/round-test.h"

static RoundTest *suite_RoundTest = 0;

static CxxTest::List Tests_RoundTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<RoundTest> suiteDescription_RoundTest( "../../src/round-test.h", 10, "RoundTest", Tests_RoundTest, suite_RoundTest, 50, 51 );

static class TestDescription_RoundTest_testNonNegRound : public CxxTest::RealTestDescription {
public:
 TestDescription_RoundTest_testNonNegRound() : CxxTest::RealTestDescription( Tests_RoundTest, suiteDescription_RoundTest, 58, "testNonNegRound" ) {}
 void runTest() { if ( suite_RoundTest ) suite_RoundTest->testNonNegRound(); }
} testDescription_RoundTest_testNonNegRound;

static class TestDescription_RoundTest_testNonPosRoung : public CxxTest::RealTestDescription {
public:
 TestDescription_RoundTest_testNonPosRoung() : CxxTest::RealTestDescription( Tests_RoundTest, suiteDescription_RoundTest, 67, "testNonPosRoung" ) {}
 void runTest() { if ( suite_RoundTest ) suite_RoundTest->testNonPosRoung(); }
} testDescription_RoundTest_testNonPosRoung;

#include "../../src/preferences-test.h"

static PreferencesTest suite_PreferencesTest;

static CxxTest::List Tests_PreferencesTest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_PreferencesTest( "../../src/preferences-test.h", 29, "PreferencesTest", suite_PreferencesTest, Tests_PreferencesTest );

static class TestDescription_PreferencesTest_testStartingState : public CxxTest::RealTestDescription {
public:
 TestDescription_PreferencesTest_testStartingState() : CxxTest::RealTestDescription( Tests_PreferencesTest, suiteDescription_PreferencesTest, 39, "testStartingState" ) {}
 void runTest() { suite_PreferencesTest.testStartingState(); }
} testDescription_PreferencesTest_testStartingState;

static class TestDescription_PreferencesTest_testOverwrite : public CxxTest::RealTestDescription {
public:
 TestDescription_PreferencesTest_testOverwrite() : CxxTest::RealTestDescription( Tests_PreferencesTest, suiteDescription_PreferencesTest, 45, "testOverwrite" ) {}
 void runTest() { suite_PreferencesTest.testOverwrite(); }
} testDescription_PreferencesTest_testOverwrite;

static class TestDescription_PreferencesTest_testDefaultReturn : public CxxTest::RealTestDescription {
public:
 TestDescription_PreferencesTest_testDefaultReturn() : CxxTest::RealTestDescription( Tests_PreferencesTest, suiteDescription_PreferencesTest, 52, "testDefaultReturn" ) {}
 void runTest() { suite_PreferencesTest.testDefaultReturn(); }
} testDescription_PreferencesTest_testDefaultReturn;

static class TestDescription_PreferencesTest_testLimitedReturn : public CxxTest::RealTestDescription {
public:
 TestDescription_PreferencesTest_testLimitedReturn() : CxxTest::RealTestDescription( Tests_PreferencesTest, suiteDescription_PreferencesTest, 57, "testLimitedReturn" ) {}
 void runTest() { suite_PreferencesTest.testLimitedReturn(); }
} testDescription_PreferencesTest_testLimitedReturn;

static class TestDescription_PreferencesTest_testKeyObserverNotification : public CxxTest::RealTestDescription {
public:
 TestDescription_PreferencesTest_testKeyObserverNotification() : CxxTest::RealTestDescription( Tests_PreferencesTest, suiteDescription_PreferencesTest, 70, "testKeyObserverNotification" ) {}
 void runTest() { suite_PreferencesTest.testKeyObserverNotification(); }
} testDescription_PreferencesTest_testKeyObserverNotification;

static class TestDescription_PreferencesTest_testEntryObserverNotification : public CxxTest::RealTestDescription {
public:
 TestDescription_PreferencesTest_testEntryObserverNotification() : CxxTest::RealTestDescription( Tests_PreferencesTest, suiteDescription_PreferencesTest, 89, "testEntryObserverNotification" ) {}
 void runTest() { suite_PreferencesTest.testEntryObserverNotification(); }
} testDescription_PreferencesTest_testEntryObserverNotification;

static class TestDescription_PreferencesTest_testPreferencesEntryMethods : public CxxTest::RealTestDescription {
public:
 TestDescription_PreferencesTest_testPreferencesEntryMethods() : CxxTest::RealTestDescription( Tests_PreferencesTest, suiteDescription_PreferencesTest, 114, "testPreferencesEntryMethods" ) {}
 void runTest() { suite_PreferencesTest.testPreferencesEntryMethods(); }
} testDescription_PreferencesTest_testPreferencesEntryMethods;

#include "../../src/sp-style-elem-test.h"

static SPStyleElemTest *suite_SPStyleElemTest = 0;

static CxxTest::List Tests_SPStyleElemTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<SPStyleElemTest> suiteDescription_SPStyleElemTest( "../../src/sp-style-elem-test.h", 12, "SPStyleElemTest", Tests_SPStyleElemTest, suite_SPStyleElemTest, 43, 48 );

static class TestDescription_SPStyleElemTest_testSetType : public CxxTest::RealTestDescription {
public:
 TestDescription_SPStyleElemTest_testSetType() : CxxTest::RealTestDescription( Tests_SPStyleElemTest, suiteDescription_SPStyleElemTest, 54, "testSetType" ) {}
 void runTest() { if ( suite_SPStyleElemTest ) suite_SPStyleElemTest->testSetType(); }
} testDescription_SPStyleElemTest_testSetType;

static class TestDescription_SPStyleElemTest_testWrite : public CxxTest::RealTestDescription {
public:
 TestDescription_SPStyleElemTest_testWrite() : CxxTest::RealTestDescription( Tests_SPStyleElemTest, suiteDescription_SPStyleElemTest, 74, "testWrite" ) {}
 void runTest() { if ( suite_SPStyleElemTest ) suite_SPStyleElemTest->testWrite(); }
} testDescription_SPStyleElemTest_testWrite;

static class TestDescription_SPStyleElemTest_testBuild : public CxxTest::RealTestDescription {
public:
 TestDescription_SPStyleElemTest_testBuild() : CxxTest::RealTestDescription( Tests_SPStyleElemTest, suiteDescription_SPStyleElemTest, 100, "testBuild" ) {}
 void runTest() { if ( suite_SPStyleElemTest ) suite_SPStyleElemTest->testBuild(); }
} testDescription_SPStyleElemTest_testBuild;

static class TestDescription_SPStyleElemTest_testReadContent : public CxxTest::RealTestDescription {
public:
 TestDescription_SPStyleElemTest_testReadContent() : CxxTest::RealTestDescription( Tests_SPStyleElemTest, suiteDescription_SPStyleElemTest, 128, "testReadContent" ) {}
 void runTest() { if ( suite_SPStyleElemTest ) suite_SPStyleElemTest->testReadContent(); }
} testDescription_SPStyleElemTest_testReadContent;

#include "../../src/style-test.h"

static StyleTest *suite_StyleTest = 0;

static CxxTest::List Tests_StyleTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<StyleTest> suiteDescription_StyleTest( "../../src/style-test.h", 11, "StyleTest", Tests_StyleTest, suite_StyleTest, 37, 43 );

static class TestDescription_StyleTest_testOne : public CxxTest::RealTestDescription {
public:
 TestDescription_StyleTest_testOne() : CxxTest::RealTestDescription( Tests_StyleTest, suiteDescription_StyleTest, 52, "testOne" ) {}
 void runTest() { if ( suite_StyleTest ) suite_StyleTest->testOne(); }
} testDescription_StyleTest_testOne;

#include "../../src/verbs-test.h"

static VerbsTest suite_VerbsTest;

static CxxTest::List Tests_VerbsTest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_VerbsTest( "../../src/verbs-test.h", 7, "VerbsTest", suite_VerbsTest, Tests_VerbsTest );

static class TestDescription_VerbsTest_testEnumLength : public CxxTest::RealTestDescription {
public:
 TestDescription_VerbsTest_testEnumLength() : CxxTest::RealTestDescription( Tests_VerbsTest, suiteDescription_VerbsTest, 19, "testEnumLength" ) {}
 void runTest() { suite_VerbsTest.testEnumLength(); }
} testDescription_VerbsTest_testEnumLength;

static class TestDescription_VerbsTest_testEnumFixed : public CxxTest::RealTestDescription {
public:
 TestDescription_VerbsTest_testEnumFixed() : CxxTest::RealTestDescription( Tests_VerbsTest, suiteDescription_VerbsTest, 25, "testEnumFixed" ) {}
 void runTest() { suite_VerbsTest.testEnumFixed(); }
} testDescription_VerbsTest_testEnumFixed;

static class TestDescription_VerbsTest_testFetch : public CxxTest::RealTestDescription {
public:
 TestDescription_VerbsTest_testFetch() : CxxTest::RealTestDescription( Tests_VerbsTest, suiteDescription_VerbsTest, 34, "testFetch" ) {}
 void runTest() { suite_VerbsTest.testFetch(); }
} testDescription_VerbsTest_testFetch;

#include "../../src/display/curve-test.h"

static CurveTest *suite_CurveTest = 0;

static CxxTest::List Tests_CurveTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<CurveTest> suiteDescription_CurveTest( "../../src/display/curve-test.h", 8, "CurveTest", Tests_CurveTest, suite_CurveTest, 35, 36 );

static class TestDescription_CurveTest_testGetSegmentCount : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testGetSegmentCount() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 38, "testGetSegmentCount" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testGetSegmentCount(); }
} testDescription_CurveTest_testGetSegmentCount;

static class TestDescription_CurveTest_testNodesInPath : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testNodesInPath() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 75, "testNodesInPath" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testNodesInPath(); }
} testDescription_CurveTest_testNodesInPath;

static class TestDescription_CurveTest_testIsEmpty : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testIsEmpty() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 112, "testIsEmpty" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testIsEmpty(); }
} testDescription_CurveTest_testIsEmpty;

static class TestDescription_CurveTest_testIsClosed : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testIsClosed() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 121, "testIsClosed" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testIsClosed(); }
} testDescription_CurveTest_testIsClosed;

static class TestDescription_CurveTest_testLastFirstSegment : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testLastFirstSegment() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 134, "testLastFirstSegment" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testLastFirstSegment(); }
} testDescription_CurveTest_testLastFirstSegment;

static class TestDescription_CurveTest_testLastFirstPath : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testLastFirstPath() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 167, "testLastFirstPath" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testLastFirstPath(); }
} testDescription_CurveTest_testLastFirstPath;

static class TestDescription_CurveTest_testFirstPoint : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testFirstPoint() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 186, "testFirstPoint" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testFirstPoint(); }
} testDescription_CurveTest_testFirstPoint;

static class TestDescription_CurveTest_testLastPoint : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testLastPoint() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 202, "testLastPoint" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testLastPoint(); }
} testDescription_CurveTest_testLastPoint;

static class TestDescription_CurveTest_testSecondPoint : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testSecondPoint() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 218, "testSecondPoint" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testSecondPoint(); }
} testDescription_CurveTest_testSecondPoint;

static class TestDescription_CurveTest_testPenultimatePoint : public CxxTest::RealTestDescription {
public:
 TestDescription_CurveTest_testPenultimatePoint() : CxxTest::RealTestDescription( Tests_CurveTest, suiteDescription_CurveTest, 233, "testPenultimatePoint" ) {}
 void runTest() { if ( suite_CurveTest ) suite_CurveTest->testPenultimatePoint(); }
} testDescription_CurveTest_testPenultimatePoint;

#include "../../src/helper/units-test.h"

static UnitsTest *suite_UnitsTest = 0;

static CxxTest::List Tests_UnitsTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<UnitsTest> suiteDescription_UnitsTest( "../../src/helper/units-test.h", 7, "UnitsTest", Tests_UnitsTest, suite_UnitsTest, 17, 18 );

static class TestDescription_UnitsTest_testConversions : public CxxTest::RealTestDescription {
public:
 TestDescription_UnitsTest_testConversions() : CxxTest::RealTestDescription( Tests_UnitsTest, suiteDescription_UnitsTest, 20, "testConversions" ) {}
 void runTest() { if ( suite_UnitsTest ) suite_UnitsTest->testConversions(); }
} testDescription_UnitsTest_testConversions;

static class TestDescription_UnitsTest_testUnitTable : public CxxTest::RealTestDescription {
public:
 TestDescription_UnitsTest_testUnitTable() : CxxTest::RealTestDescription( Tests_UnitsTest, suiteDescription_UnitsTest, 54, "testUnitTable" ) {}
 void runTest() { if ( suite_UnitsTest ) suite_UnitsTest->testUnitTable(); }
} testDescription_UnitsTest_testUnitTable;

#include "../../src/libnr/in-svg-plane-test.h"

static InSvgPlaneTest *suite_InSvgPlaneTest = 0;

static CxxTest::List Tests_InSvgPlaneTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<InSvgPlaneTest> suiteDescription_InSvgPlaneTest( "../../src/libnr/in-svg-plane-test.h", 9, "InSvgPlaneTest", Tests_InSvgPlaneTest, suite_InSvgPlaneTest, 31, 32 );

static class TestDescription_InSvgPlaneTest_testInSvgPlane : public CxxTest::RealTestDescription {
public:
 TestDescription_InSvgPlaneTest_testInSvgPlane() : CxxTest::RealTestDescription( Tests_InSvgPlaneTest, suiteDescription_InSvgPlaneTest, 51, "testInSvgPlane" ) {}
 void runTest() { if ( suite_InSvgPlaneTest ) suite_InSvgPlaneTest->testInSvgPlane(); }
} testDescription_InSvgPlaneTest_testInSvgPlane;

#include "../../src/libnr/nr-compose-test.h"

static NrComposeTest *suite_NrComposeTest = 0;

static CxxTest::List Tests_NrComposeTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<NrComposeTest> suiteDescription_NrComposeTest( "../../src/libnr/nr-compose-test.h", 35, "NrComposeTest", Tests_NrComposeTest, suite_NrComposeTest, 110, 111 );

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 115, "testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 129, "testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 143, "testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 157, "testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 171, "testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 185, "testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 199, "testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 213, "testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 229, "testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_N_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 238, "testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_R8G8B8A8_P_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 247, "testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_N_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 256, "testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_R8G8B8A8_P_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 265, "testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 274, "testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_P_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 283, "testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 292, "testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_P_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_A8_RGBA32 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_A8_RGBA32() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 303, "testnr_R8G8B8A8_N_EMPTY_A8_RGBA32" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_EMPTY_A8_RGBA32(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_EMPTY_A8_RGBA32;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_A8_RGBA32 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_A8_RGBA32() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 319, "testnr_R8G8B8A8_P_EMPTY_A8_RGBA32" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_EMPTY_A8_RGBA32(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_EMPTY_A8_RGBA32;

static class TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_A8_RGBA32 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_A8_RGBA32() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 335, "testnr_R8G8B8_R8G8B8_A8_RGBA32" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8_R8G8B8_A8_RGBA32(); }
} testDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_A8_RGBA32;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 351, "testnr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_N_R8G8B8A8_N_A8_RGBA32;

static class TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 367, "testnr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32(); }
} testDescription_NrComposeTest_testnr_R8G8B8A8_P_R8G8B8A8_P_A8_RGBA32;

static class TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_N : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_N() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 385, "testnr_R8G8B8_R8G8B8_R8G8B8A8_N" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8_R8G8B8_R8G8B8A8_N(); }
} testDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_N;

static class TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_P : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_P() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 399, "testnr_R8G8B8_R8G8B8_R8G8B8A8_P" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8_R8G8B8_R8G8B8A8_P(); }
} testDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_P;

static class TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_N_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_N_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 413, "testnr_R8G8B8_R8G8B8_R8G8B8A8_N_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8_R8G8B8_R8G8B8A8_N_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_N_A8;

static class TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_P_A8 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_P_A8() : CxxTest::RealTestDescription( Tests_NrComposeTest, suiteDescription_NrComposeTest, 427, "testnr_R8G8B8_R8G8B8_R8G8B8A8_P_A8" ) {}
 void runTest() { if ( suite_NrComposeTest ) suite_NrComposeTest->testnr_R8G8B8_R8G8B8_R8G8B8A8_P_A8(); }
} testDescription_NrComposeTest_testnr_R8G8B8_R8G8B8_R8G8B8A8_P_A8;

#include "../../src/libnr/nr-matrix-test.h"

static NrMatrixTest *suite_NrMatrixTest = 0;

static CxxTest::List Tests_NrMatrixTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<NrMatrixTest> suiteDescription_NrMatrixTest( "../../src/libnr/nr-matrix-test.h", 23, "NrMatrixTest", Tests_NrMatrixTest, suite_NrMatrixTest, 44, 45 );

static class TestDescription_NrMatrixTest_testCtorsAssignmentOp : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testCtorsAssignmentOp() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 59, "testCtorsAssignmentOp" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testCtorsAssignmentOp(); }
} testDescription_NrMatrixTest_testCtorsAssignmentOp;

static class TestDescription_NrMatrixTest_testScaleCtor : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testScaleCtor() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 72, "testScaleCtor" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testScaleCtor(); }
} testDescription_NrMatrixTest_testScaleCtor;

static class TestDescription_NrMatrixTest_testRotateCtor : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testRotateCtor() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 81, "testRotateCtor" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testRotateCtor(); }
} testDescription_NrMatrixTest_testRotateCtor;

static class TestDescription_NrMatrixTest_testTranslateCtor : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testTranslateCtor() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 94, "testTranslateCtor" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testTranslateCtor(); }
} testDescription_NrMatrixTest_testTranslateCtor;

static class TestDescription_NrMatrixTest_testIdentity : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testIdentity() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 101, "testIdentity" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testIdentity(); }
} testDescription_NrMatrixTest_testIdentity;

static class TestDescription_NrMatrixTest_testInverse : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testInverse() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 116, "testInverse" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testInverse(); }
} testDescription_NrMatrixTest_testInverse;

static class TestDescription_NrMatrixTest_testEllipticQuadraticForm : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testEllipticQuadraticForm() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 126, "testEllipticQuadraticForm" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testEllipticQuadraticForm(); }
} testDescription_NrMatrixTest_testEllipticQuadraticForm;

static class TestDescription_NrMatrixTest_testMatrixStarRotate : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testMatrixStarRotate() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 141, "testMatrixStarRotate" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testMatrixStarRotate(); }
} testDescription_NrMatrixTest_testMatrixStarRotate;

static class TestDescription_NrMatrixTest_testTranslateStarScale_ScaleStarTranslate : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testTranslateStarScale_ScaleStarTranslate() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 155, "testTranslateStarScale_ScaleStarTranslate" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testTranslateStarScale_ScaleStarTranslate(); }
} testDescription_NrMatrixTest_testTranslateStarScale_ScaleStarTranslate;

static class TestDescription_NrMatrixTest_testMatrixStarScale : public CxxTest::RealTestDescription {
public:
 TestDescription_NrMatrixTest_testMatrixStarScale() : CxxTest::RealTestDescription( Tests_NrMatrixTest, suiteDescription_NrMatrixTest, 169, "testMatrixStarScale" ) {}
 void runTest() { if ( suite_NrMatrixTest ) suite_NrMatrixTest->testMatrixStarScale(); }
} testDescription_NrMatrixTest_testMatrixStarScale;

#include "../../src/libnr/nr-point-fns-test.h"

static NrPointFnsTest *suite_NrPointFnsTest = 0;

static CxxTest::List Tests_NrPointFnsTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<NrPointFnsTest> suiteDescription_NrPointFnsTest( "../../src/libnr/nr-point-fns-test.h", 12, "NrPointFnsTest", Tests_NrPointFnsTest, suite_NrPointFnsTest, 37, 38 );

static class TestDescription_NrPointFnsTest_testL1 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrPointFnsTest_testL1() : CxxTest::RealTestDescription( Tests_NrPointFnsTest, suiteDescription_NrPointFnsTest, 59, "testL1" ) {}
 void runTest() { if ( suite_NrPointFnsTest ) suite_NrPointFnsTest->testL1(); }
} testDescription_NrPointFnsTest_testL1;

static class TestDescription_NrPointFnsTest_testL2 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrPointFnsTest_testL2() : CxxTest::RealTestDescription( Tests_NrPointFnsTest, suiteDescription_NrPointFnsTest, 69, "testL2" ) {}
 void runTest() { if ( suite_NrPointFnsTest ) suite_NrPointFnsTest->testL2(); }
} testDescription_NrPointFnsTest_testL2;

static class TestDescription_NrPointFnsTest_testLInfty : public CxxTest::RealTestDescription {
public:
 TestDescription_NrPointFnsTest_testLInfty() : CxxTest::RealTestDescription( Tests_NrPointFnsTest, suiteDescription_NrPointFnsTest, 79, "testLInfty" ) {}
 void runTest() { if ( suite_NrPointFnsTest ) suite_NrPointFnsTest->testLInfty(); }
} testDescription_NrPointFnsTest_testLInfty;

static class TestDescription_NrPointFnsTest_testIsZero : public CxxTest::RealTestDescription {
public:
 TestDescription_NrPointFnsTest_testIsZero() : CxxTest::RealTestDescription( Tests_NrPointFnsTest, suiteDescription_NrPointFnsTest, 89, "testIsZero" ) {}
 void runTest() { if ( suite_NrPointFnsTest ) suite_NrPointFnsTest->testIsZero(); }
} testDescription_NrPointFnsTest_testIsZero;

static class TestDescription_NrPointFnsTest_testAtan2 : public CxxTest::RealTestDescription {
public:
 TestDescription_NrPointFnsTest_testAtan2() : CxxTest::RealTestDescription( Tests_NrPointFnsTest, suiteDescription_NrPointFnsTest, 99, "testAtan2" ) {}
 void runTest() { if ( suite_NrPointFnsTest ) suite_NrPointFnsTest->testAtan2(); }
} testDescription_NrPointFnsTest_testAtan2;

static class TestDescription_NrPointFnsTest_testUnitVector : public CxxTest::RealTestDescription {
public:
 TestDescription_NrPointFnsTest_testUnitVector() : CxxTest::RealTestDescription( Tests_NrPointFnsTest, suiteDescription_NrPointFnsTest, 106, "testUnitVector" ) {}
 void runTest() { if ( suite_NrPointFnsTest ) suite_NrPointFnsTest->testUnitVector(); }
} testDescription_NrPointFnsTest_testUnitVector;

static class TestDescription_NrPointFnsTest_testIsUnitVector : public CxxTest::RealTestDescription {
public:
 TestDescription_NrPointFnsTest_testIsUnitVector() : CxxTest::RealTestDescription( Tests_NrPointFnsTest, suiteDescription_NrPointFnsTest, 113, "testIsUnitVector" ) {}
 void runTest() { if ( suite_NrPointFnsTest ) suite_NrPointFnsTest->testIsUnitVector(); }
} testDescription_NrPointFnsTest_testIsUnitVector;

#include "../../src/libnr/nr-rotate-test.h"

static NrRotateTest *suite_NrRotateTest = 0;

static CxxTest::List Tests_NrRotateTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<NrRotateTest> suiteDescription_NrRotateTest( "../../src/libnr/nr-rotate-test.h", 13, "NrRotateTest", Tests_NrRotateTest, suite_NrRotateTest, 29, 30 );

static class TestDescription_NrRotateTest_testCtorsCompares : public CxxTest::RealTestDescription {
public:
 TestDescription_NrRotateTest_testCtorsCompares() : CxxTest::RealTestDescription( Tests_NrRotateTest, suiteDescription_NrRotateTest, 41, "testCtorsCompares" ) {}
 void runTest() { if ( suite_NrRotateTest ) suite_NrRotateTest->testCtorsCompares(); }
} testDescription_NrRotateTest_testCtorsCompares;

static class TestDescription_NrRotateTest_testAssignmentOp : public CxxTest::RealTestDescription {
public:
 TestDescription_NrRotateTest_testAssignmentOp() : CxxTest::RealTestDescription( Tests_NrRotateTest, suiteDescription_NrRotateTest, 53, "testAssignmentOp" ) {}
 void runTest() { if ( suite_NrRotateTest ) suite_NrRotateTest->testAssignmentOp(); }
} testDescription_NrRotateTest_testAssignmentOp;

static class TestDescription_NrRotateTest_testInverse : public CxxTest::RealTestDescription {
public:
 TestDescription_NrRotateTest_testInverse() : CxxTest::RealTestDescription( Tests_NrRotateTest, suiteDescription_NrRotateTest, 61, "testInverse" ) {}
 void runTest() { if ( suite_NrRotateTest ) suite_NrRotateTest->testInverse(); }
} testDescription_NrRotateTest_testInverse;

static class TestDescription_NrRotateTest_testOpStarPointRotate : public CxxTest::RealTestDescription {
public:
 TestDescription_NrRotateTest_testOpStarPointRotate() : CxxTest::RealTestDescription( Tests_NrRotateTest, suiteDescription_NrRotateTest, 67, "testOpStarPointRotate" ) {}
 void runTest() { if ( suite_NrRotateTest ) suite_NrRotateTest->testOpStarPointRotate(); }
} testDescription_NrRotateTest_testOpStarPointRotate;

static class TestDescription_NrRotateTest_testOpStarRotateRotate : public CxxTest::RealTestDescription {
public:
 TestDescription_NrRotateTest_testOpStarRotateRotate() : CxxTest::RealTestDescription( Tests_NrRotateTest, suiteDescription_NrRotateTest, 78, "testOpStarRotateRotate" ) {}
 void runTest() { if ( suite_NrRotateTest ) suite_NrRotateTest->testOpStarRotateRotate(); }
} testDescription_NrRotateTest_testOpStarRotateRotate;

static class TestDescription_NrRotateTest_testOpDivRotateRotate : public CxxTest::RealTestDescription {
public:
 TestDescription_NrRotateTest_testOpDivRotateRotate() : CxxTest::RealTestDescription( Tests_NrRotateTest, suiteDescription_NrRotateTest, 91, "testOpDivRotateRotate" ) {}
 void runTest() { if ( suite_NrRotateTest ) suite_NrRotateTest->testOpDivRotateRotate(); }
} testDescription_NrRotateTest_testOpDivRotateRotate;

#include "../../src/libnr/nr-rotate-fns-test.h"

static NrRotateFnsTest *suite_NrRotateFnsTest = 0;

static CxxTest::List Tests_NrRotateFnsTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<NrRotateFnsTest> suiteDescription_NrRotateFnsTest( "../../src/libnr/nr-rotate-fns-test.h", 8, "NrRotateFnsTest", Tests_NrRotateFnsTest, suite_NrRotateFnsTest, 19, 20 );

static class TestDescription_NrRotateFnsTest_testRotateDegrees : public CxxTest::RealTestDescription {
public:
 TestDescription_NrRotateFnsTest_testRotateDegrees() : CxxTest::RealTestDescription( Tests_NrRotateFnsTest, suiteDescription_NrRotateFnsTest, 24, "testRotateDegrees" ) {}
 void runTest() { if ( suite_NrRotateFnsTest ) suite_NrRotateFnsTest->testRotateDegrees(); }
} testDescription_NrRotateFnsTest_testRotateDegrees;

#include "../../src/libnr/nr-scale-test.h"

static NrScaleTest *suite_NrScaleTest = 0;

static CxxTest::List Tests_NrScaleTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<NrScaleTest> suiteDescription_NrScaleTest( "../../src/libnr/nr-scale-test.h", 6, "NrScaleTest", Tests_NrScaleTest, suite_NrScaleTest, 20, 21 );

static class TestDescription_NrScaleTest_testXY_CtorArrayOperator : public CxxTest::RealTestDescription {
public:
 TestDescription_NrScaleTest_testXY_CtorArrayOperator() : CxxTest::RealTestDescription( Tests_NrScaleTest, suiteDescription_NrScaleTest, 29, "testXY_CtorArrayOperator" ) {}
 void runTest() { if ( suite_NrScaleTest ) suite_NrScaleTest->testXY_CtorArrayOperator(); }
} testDescription_NrScaleTest_testXY_CtorArrayOperator;

static class TestDescription_NrScaleTest_testCopyCtor_AssignmentOp_NotEquals : public CxxTest::RealTestDescription {
public:
 TestDescription_NrScaleTest_testCopyCtor_AssignmentOp_NotEquals() : CxxTest::RealTestDescription( Tests_NrScaleTest, suiteDescription_NrScaleTest, 38, "testCopyCtor_AssignmentOp_NotEquals" ) {}
 void runTest() { if ( suite_NrScaleTest ) suite_NrScaleTest->testCopyCtor_AssignmentOp_NotEquals(); }
} testDescription_NrScaleTest_testCopyCtor_AssignmentOp_NotEquals;

static class TestDescription_NrScaleTest_testAssignmentOp : public CxxTest::RealTestDescription {
public:
 TestDescription_NrScaleTest_testAssignmentOp() : CxxTest::RealTestDescription( Tests_NrScaleTest, suiteDescription_NrScaleTest, 46, "testAssignmentOp" ) {}
 void runTest() { if ( suite_NrScaleTest ) suite_NrScaleTest->testAssignmentOp(); }
} testDescription_NrScaleTest_testAssignmentOp;

static class TestDescription_NrScaleTest_testPointCtor : public CxxTest::RealTestDescription {
public:
 TestDescription_NrScaleTest_testPointCtor() : CxxTest::RealTestDescription( Tests_NrScaleTest, suiteDescription_NrScaleTest, 53, "testPointCtor" ) {}
 void runTest() { if ( suite_NrScaleTest ) suite_NrScaleTest->testPointCtor(); }
} testDescription_NrScaleTest_testPointCtor;

static class TestDescription_NrScaleTest_testOpStarPointScale : public CxxTest::RealTestDescription {
public:
 TestDescription_NrScaleTest_testOpStarPointScale() : CxxTest::RealTestDescription( Tests_NrScaleTest, suiteDescription_NrScaleTest, 59, "testOpStarPointScale" ) {}
 void runTest() { if ( suite_NrScaleTest ) suite_NrScaleTest->testOpStarPointScale(); }
} testDescription_NrScaleTest_testOpStarPointScale;

static class TestDescription_NrScaleTest_testOpStarScaleScale : public CxxTest::RealTestDescription {
public:
 TestDescription_NrScaleTest_testOpStarScaleScale() : CxxTest::RealTestDescription( Tests_NrScaleTest, suiteDescription_NrScaleTest, 65, "testOpStarScaleScale" ) {}
 void runTest() { if ( suite_NrScaleTest ) suite_NrScaleTest->testOpStarScaleScale(); }
} testDescription_NrScaleTest_testOpStarScaleScale;

static class TestDescription_NrScaleTest_testOpDivScaleScale : public CxxTest::RealTestDescription {
public:
 TestDescription_NrScaleTest_testOpDivScaleScale() : CxxTest::RealTestDescription( Tests_NrScaleTest, suiteDescription_NrScaleTest, 71, "testOpDivScaleScale" ) {}
 void runTest() { if ( suite_NrScaleTest ) suite_NrScaleTest->testOpDivScaleScale(); }
} testDescription_NrScaleTest_testOpDivScaleScale;

#include "../../src/libnr/nr-translate-test.h"

static NrTranslateTest *suite_NrTranslateTest = 0;

static CxxTest::List Tests_NrTranslateTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<NrTranslateTest> suiteDescription_NrTranslateTest( "../../src/libnr/nr-translate-test.h", 11, "NrTranslateTest", Tests_NrTranslateTest, suite_NrTranslateTest, 28, 29 );

static class TestDescription_NrTranslateTest_testCtorsArrayOperator : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTranslateTest_testCtorsArrayOperator() : CxxTest::RealTestDescription( Tests_NrTranslateTest, suiteDescription_NrTranslateTest, 39, "testCtorsArrayOperator" ) {}
 void runTest() { if ( suite_NrTranslateTest ) suite_NrTranslateTest->testCtorsArrayOperator(); }
} testDescription_NrTranslateTest_testCtorsArrayOperator;

static class TestDescription_NrTranslateTest_testAssignmentOperator : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTranslateTest_testAssignmentOperator() : CxxTest::RealTestDescription( Tests_NrTranslateTest, suiteDescription_NrTranslateTest, 48, "testAssignmentOperator" ) {}
 void runTest() { if ( suite_NrTranslateTest ) suite_NrTranslateTest->testAssignmentOperator(); }
} testDescription_NrTranslateTest_testAssignmentOperator;

static class TestDescription_NrTranslateTest_testOpStarTranslateTranslate : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTranslateTest_testOpStarTranslateTranslate() : CxxTest::RealTestDescription( Tests_NrTranslateTest, suiteDescription_NrTranslateTest, 56, "testOpStarTranslateTranslate" ) {}
 void runTest() { if ( suite_NrTranslateTest ) suite_NrTranslateTest->testOpStarTranslateTranslate(); }
} testDescription_NrTranslateTest_testOpStarTranslateTranslate;

static class TestDescription_NrTranslateTest_testOpStarPointTranslate : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTranslateTest_testOpStarPointTranslate() : CxxTest::RealTestDescription( Tests_NrTranslateTest, suiteDescription_NrTranslateTest, 63, "testOpStarPointTranslate" ) {}
 void runTest() { if ( suite_NrTranslateTest ) suite_NrTranslateTest->testOpStarPointTranslate(); }
} testDescription_NrTranslateTest_testOpStarPointTranslate;

static class TestDescription_NrTranslateTest_testIdentity : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTranslateTest_testIdentity() : CxxTest::RealTestDescription( Tests_NrTranslateTest, suiteDescription_NrTranslateTest, 69, "testIdentity" ) {}
 void runTest() { if ( suite_NrTranslateTest ) suite_NrTranslateTest->testIdentity(); }
} testDescription_NrTranslateTest_testIdentity;

#include "../../src/libnr/nr-types-test.h"

static NrTypesTest *suite_NrTypesTest = 0;

static CxxTest::List Tests_NrTypesTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<NrTypesTest> suiteDescription_NrTypesTest( "../../src/libnr/nr-types-test.h", 8, "NrTypesTest", Tests_NrTypesTest, suite_NrTypesTest, 23, 24 );

static class TestDescription_NrTypesTest_testXYValues : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testXYValues() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 34, "testXYValues" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testXYValues(); }
} testDescription_NrTypesTest_testXYValues;

static class TestDescription_NrTypesTest_testXYCtorAndArrayConst : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testXYCtorAndArrayConst() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 40, "testXYCtorAndArrayConst" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testXYCtorAndArrayConst(); }
} testDescription_NrTypesTest_testXYCtorAndArrayConst;

static class TestDescription_NrTypesTest_testCopyCtor : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testCopyCtor() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 46, "testCopyCtor" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testCopyCtor(); }
} testDescription_NrTypesTest_testCopyCtor;

static class TestDescription_NrTypesTest_testNonConstArrayOperator : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testNonConstArrayOperator() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 54, "testNonConstArrayOperator" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testNonConstArrayOperator(); }
} testDescription_NrTypesTest_testNonConstArrayOperator;

static class TestDescription_NrTypesTest_testBinaryPlusMinus : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testBinaryPlusMinus() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 64, "testBinaryPlusMinus" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testBinaryPlusMinus(); }
} testDescription_NrTypesTest_testBinaryPlusMinus;

static class TestDescription_NrTypesTest_testUnaryMinus : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testUnaryMinus() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 73, "testUnaryMinus" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testUnaryMinus(); }
} testDescription_NrTypesTest_testUnaryMinus;

static class TestDescription_NrTypesTest_testDot : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testDot() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 85, "testDot" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testDot(); }
} testDescription_NrTypesTest_testDot;

static class TestDescription_NrTypesTest_testL1L2LInftyNorms : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testL1L2LInftyNorms() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 93, "testL1L2LInftyNorms" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testL1L2LInftyNorms(); }
} testDescription_NrTypesTest_testL1L2LInftyNorms;

static class TestDescription_NrTypesTest_testOperatorPlusEquals : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testOperatorPlusEquals() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 106, "testOperatorPlusEquals" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testOperatorPlusEquals(); }
} testDescription_NrTypesTest_testOperatorPlusEquals;

static class TestDescription_NrTypesTest_testNormalize : public CxxTest::RealTestDescription {
public:
 TestDescription_NrTypesTest_testNormalize() : CxxTest::RealTestDescription( Tests_NrTypesTest, suiteDescription_NrTypesTest, 120, "testNormalize" ) {}
 void runTest() { if ( suite_NrTypesTest ) suite_NrTypesTest->testNormalize(); }
} testDescription_NrTypesTest_testNormalize;

#include "../../src/svg/css-ostringstream-test.h"

static CSSOStringStreamTest *suite_CSSOStringStreamTest = 0;

static CxxTest::List Tests_CSSOStringStreamTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<CSSOStringStreamTest> suiteDescription_CSSOStringStreamTest( "../../src/svg/css-ostringstream-test.h", 20, "CSSOStringStreamTest", Tests_CSSOStringStreamTest, suite_CSSOStringStreamTest, 26, 27 );

static class TestDescription_CSSOStringStreamTest_testFloats : public CxxTest::RealTestDescription {
public:
 TestDescription_CSSOStringStreamTest_testFloats() : CxxTest::RealTestDescription( Tests_CSSOStringStreamTest, suiteDescription_CSSOStringStreamTest, 29, "testFloats" ) {}
 void runTest() { if ( suite_CSSOStringStreamTest ) suite_CSSOStringStreamTest->testFloats(); }
} testDescription_CSSOStringStreamTest_testFloats;

static class TestDescription_CSSOStringStreamTest_testOtherTypes : public CxxTest::RealTestDescription {
public:
 TestDescription_CSSOStringStreamTest_testOtherTypes() : CxxTest::RealTestDescription( Tests_CSSOStringStreamTest, suiteDescription_CSSOStringStreamTest, 48, "testOtherTypes" ) {}
 void runTest() { if ( suite_CSSOStringStreamTest ) suite_CSSOStringStreamTest->testOtherTypes(); }
} testDescription_CSSOStringStreamTest_testOtherTypes;

static class TestDescription_CSSOStringStreamTest_testConcat : public CxxTest::RealTestDescription {
public:
 TestDescription_CSSOStringStreamTest_testConcat() : CxxTest::RealTestDescription( Tests_CSSOStringStreamTest, suiteDescription_CSSOStringStreamTest, 59, "testConcat" ) {}
 void runTest() { if ( suite_CSSOStringStreamTest ) suite_CSSOStringStreamTest->testConcat(); }
} testDescription_CSSOStringStreamTest_testConcat;

#include "../../src/svg/stringstream-test.h"

static StringStreamTest *suite_StringStreamTest = 0;

static CxxTest::List Tests_StringStreamTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<StringStreamTest> suiteDescription_StringStreamTest( "../../src/svg/stringstream-test.h", 21, "StringStreamTest", Tests_StringStreamTest, suite_StringStreamTest, 27, 28 );

static class TestDescription_StringStreamTest_testFloats : public CxxTest::RealTestDescription {
public:
 TestDescription_StringStreamTest_testFloats() : CxxTest::RealTestDescription( Tests_StringStreamTest, suiteDescription_StringStreamTest, 30, "testFloats" ) {}
 void runTest() { if ( suite_StringStreamTest ) suite_StringStreamTest->testFloats(); }
} testDescription_StringStreamTest_testFloats;

static class TestDescription_StringStreamTest_testOtherTypes : public CxxTest::RealTestDescription {
public:
 TestDescription_StringStreamTest_testOtherTypes() : CxxTest::RealTestDescription( Tests_StringStreamTest, suiteDescription_StringStreamTest, 46, "testOtherTypes" ) {}
 void runTest() { if ( suite_StringStreamTest ) suite_StringStreamTest->testOtherTypes(); }
} testDescription_StringStreamTest_testOtherTypes;

static class TestDescription_StringStreamTest_testConcat : public CxxTest::RealTestDescription {
public:
 TestDescription_StringStreamTest_testConcat() : CxxTest::RealTestDescription( Tests_StringStreamTest, suiteDescription_StringStreamTest, 58, "testConcat" ) {}
 void runTest() { if ( suite_StringStreamTest ) suite_StringStreamTest->testConcat(); }
} testDescription_StringStreamTest_testConcat;

#include "../../src/svg/svg-affine-test.h"

static SvgAffineTest *suite_SvgAffineTest = 0;

static CxxTest::List Tests_SvgAffineTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<SvgAffineTest> suiteDescription_SvgAffineTest( "../../src/svg/svg-affine-test.h", 12, "SvgAffineTest", Tests_SvgAffineTest, suite_SvgAffineTest, 46, 47 );

static class TestDescription_SvgAffineTest_testReadIdentity : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testReadIdentity() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 49, "testReadIdentity" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testReadIdentity(); }
} testDescription_SvgAffineTest_testReadIdentity;

static class TestDescription_SvgAffineTest_testWriteIdentity : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testWriteIdentity() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 68, "testWriteIdentity" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testWriteIdentity(); }
} testDescription_SvgAffineTest_testWriteIdentity;

static class TestDescription_SvgAffineTest_testReadMatrix : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testReadMatrix() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 73, "testReadMatrix" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testReadMatrix(); }
} testDescription_SvgAffineTest_testReadMatrix;

static class TestDescription_SvgAffineTest_testReadTranslate : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testReadTranslate() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 82, "testReadTranslate" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testReadTranslate(); }
} testDescription_SvgAffineTest_testReadTranslate;

static class TestDescription_SvgAffineTest_testReadScale : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testReadScale() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 91, "testReadScale" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testReadScale(); }
} testDescription_SvgAffineTest_testReadScale;

static class TestDescription_SvgAffineTest_testReadRotate : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testReadRotate() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 100, "testReadRotate" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testReadRotate(); }
} testDescription_SvgAffineTest_testReadRotate;

static class TestDescription_SvgAffineTest_testReadSkew : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testReadSkew() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 109, "testReadSkew" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testReadSkew(); }
} testDescription_SvgAffineTest_testReadSkew;

static class TestDescription_SvgAffineTest_testWriteMatrix : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testWriteMatrix() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 118, "testWriteMatrix" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testWriteMatrix(); }
} testDescription_SvgAffineTest_testWriteMatrix;

static class TestDescription_SvgAffineTest_testWriteTranslate : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testWriteTranslate() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 127, "testWriteTranslate" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testWriteTranslate(); }
} testDescription_SvgAffineTest_testWriteTranslate;

static class TestDescription_SvgAffineTest_testWriteScale : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testWriteScale() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 136, "testWriteScale" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testWriteScale(); }
} testDescription_SvgAffineTest_testWriteScale;

static class TestDescription_SvgAffineTest_testWriteRotate : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testWriteRotate() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 145, "testWriteRotate" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testWriteRotate(); }
} testDescription_SvgAffineTest_testWriteRotate;

static class TestDescription_SvgAffineTest_testWriteSkew : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testWriteSkew() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 154, "testWriteSkew" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testWriteSkew(); }
} testDescription_SvgAffineTest_testWriteSkew;

static class TestDescription_SvgAffineTest_testReadConcatenation : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testReadConcatenation() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 163, "testReadConcatenation" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testReadConcatenation(); }
} testDescription_SvgAffineTest_testReadConcatenation;

static class TestDescription_SvgAffineTest_testReadFailures : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgAffineTest_testReadFailures() : CxxTest::RealTestDescription( Tests_SvgAffineTest, suiteDescription_SvgAffineTest, 175, "testReadFailures" ) {}
 void runTest() { if ( suite_SvgAffineTest ) suite_SvgAffineTest->testReadFailures(); }
} testDescription_SvgAffineTest_testReadFailures;

#include "../../src/svg/svg-color-test.h"

static SVGColorTest *suite_SVGColorTest = 0;

static CxxTest::List Tests_SVGColorTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<SVGColorTest> suiteDescription_SVGColorTest( "../../src/svg/svg-color-test.h", 9, "SVGColorTest", Tests_SVGColorTest, suite_SVGColorTest, 35, 36 );

static class TestDescription_SVGColorTest_testWrite : public CxxTest::RealTestDescription {
public:
 TestDescription_SVGColorTest_testWrite() : CxxTest::RealTestDescription( Tests_SVGColorTest, suiteDescription_SVGColorTest, 38, "testWrite" ) {}
 void runTest() { if ( suite_SVGColorTest ) suite_SVGColorTest->testWrite(); }
} testDescription_SVGColorTest_testWrite;

static class TestDescription_SVGColorTest_testReadColor : public CxxTest::RealTestDescription {
public:
 TestDescription_SVGColorTest_testReadColor() : CxxTest::RealTestDescription( Tests_SVGColorTest, suiteDescription_SVGColorTest, 61, "testReadColor" ) {}
 void runTest() { if ( suite_SVGColorTest ) suite_SVGColorTest->testReadColor(); }
} testDescription_SVGColorTest_testReadColor;

static class TestDescription_SVGColorTest_testIccColor : public CxxTest::RealTestDescription {
public:
 TestDescription_SVGColorTest_testIccColor() : CxxTest::RealTestDescription( Tests_SVGColorTest, suiteDescription_SVGColorTest, 73, "testIccColor" ) {}
 void runTest() { if ( suite_SVGColorTest ) suite_SVGColorTest->testIccColor(); }
} testDescription_SVGColorTest_testIccColor;

#include "../../src/svg/svg-length-test.h"

static SvgLengthTest *suite_SvgLengthTest = 0;

static CxxTest::List Tests_SvgLengthTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<SvgLengthTest> suiteDescription_SvgLengthTest( "../../src/svg/svg-length-test.h", 10, "SvgLengthTest", Tests_SvgLengthTest, suite_SvgLengthTest, 29, 30 );

static class TestDescription_SvgLengthTest_testRead : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgLengthTest_testRead() : CxxTest::RealTestDescription( Tests_SvgLengthTest, suiteDescription_SvgLengthTest, 32, "testRead" ) {}
 void runTest() { if ( suite_SvgLengthTest ) suite_SvgLengthTest->testRead(); }
} testDescription_SvgLengthTest_testRead;

static class TestDescription_SvgLengthTest_testReadOrUnset : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgLengthTest_testReadOrUnset() : CxxTest::RealTestDescription( Tests_SvgLengthTest, suiteDescription_SvgLengthTest, 55, "testReadOrUnset" ) {}
 void runTest() { if ( suite_SvgLengthTest ) suite_SvgLengthTest->testReadOrUnset(); }
} testDescription_SvgLengthTest_testReadOrUnset;

static class TestDescription_SvgLengthTest_testReadAbsolute : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgLengthTest_testReadAbsolute() : CxxTest::RealTestDescription( Tests_SvgLengthTest, suiteDescription_SvgLengthTest, 81, "testReadAbsolute" ) {}
 void runTest() { if ( suite_SvgLengthTest ) suite_SvgLengthTest->testReadAbsolute(); }
} testDescription_SvgLengthTest_testReadAbsolute;

static class TestDescription_SvgLengthTest_testEnumMappedToString : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgLengthTest_testEnumMappedToString() : CxxTest::RealTestDescription( Tests_SvgLengthTest, suiteDescription_SvgLengthTest, 100, "testEnumMappedToString" ) {}
 void runTest() { if ( suite_SvgLengthTest ) suite_SvgLengthTest->testEnumMappedToString(); }
} testDescription_SvgLengthTest_testEnumMappedToString;

static class TestDescription_SvgLengthTest_testStringsAreValidSVG : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgLengthTest_testStringsAreValidSVG() : CxxTest::RealTestDescription( Tests_SvgLengthTest, suiteDescription_SvgLengthTest, 113, "testStringsAreValidSVG" ) {}
 void runTest() { if ( suite_SvgLengthTest ) suite_SvgLengthTest->testStringsAreValidSVG(); }
} testDescription_SvgLengthTest_testStringsAreValidSVG;

static class TestDescription_SvgLengthTest_testValidSVGStringsSupported : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgLengthTest_testValidSVGStringsSupported() : CxxTest::RealTestDescription( Tests_SvgLengthTest, suiteDescription_SvgLengthTest, 125, "testValidSVGStringsSupported" ) {}
 void runTest() { if ( suite_SvgLengthTest ) suite_SvgLengthTest->testValidSVGStringsSupported(); }
} testDescription_SvgLengthTest_testValidSVGStringsSupported;

static class TestDescription_SvgLengthTest_testPlaces : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgLengthTest_testPlaces() : CxxTest::RealTestDescription( Tests_SvgLengthTest, suiteDescription_SvgLengthTest, 141, "testPlaces" ) {}
 void runTest() { if ( suite_SvgLengthTest ) suite_SvgLengthTest->testPlaces(); }
} testDescription_SvgLengthTest_testPlaces;

#include "../../src/svg/svg-path-geom-test.h"

static SvgPathGeomTest *suite_SvgPathGeomTest = 0;

static CxxTest::List Tests_SvgPathGeomTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<SvgPathGeomTest> suiteDescription_SvgPathGeomTest( "../../src/svg/svg-path-geom-test.h", 13, "SvgPathGeomTest", Tests_SvgPathGeomTest, suite_SvgPathGeomTest, 69, 70 );

static class TestDescription_SvgPathGeomTest_testReadRectanglesAbsoluteClosed : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadRectanglesAbsoluteClosed() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 72, "testReadRectanglesAbsoluteClosed" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadRectanglesAbsoluteClosed(); }
} testDescription_SvgPathGeomTest_testReadRectanglesAbsoluteClosed;

static class TestDescription_SvgPathGeomTest_testReadRectanglesRelativeClosed : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadRectanglesRelativeClosed() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 80, "testReadRectanglesRelativeClosed" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadRectanglesRelativeClosed(); }
} testDescription_SvgPathGeomTest_testReadRectanglesRelativeClosed;

static class TestDescription_SvgPathGeomTest_testReadRectanglesAbsoluteOpen : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadRectanglesAbsoluteOpen() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 88, "testReadRectanglesAbsoluteOpen" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadRectanglesAbsoluteOpen(); }
} testDescription_SvgPathGeomTest_testReadRectanglesAbsoluteOpen;

static class TestDescription_SvgPathGeomTest_testReadRectanglesRelativeOpen : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadRectanglesRelativeOpen() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 96, "testReadRectanglesRelativeOpen" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadRectanglesRelativeOpen(); }
} testDescription_SvgPathGeomTest_testReadRectanglesRelativeOpen;

static class TestDescription_SvgPathGeomTest_testReadRectanglesAbsoluteClosed2 : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadRectanglesAbsoluteClosed2() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 104, "testReadRectanglesAbsoluteClosed2" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadRectanglesAbsoluteClosed2(); }
} testDescription_SvgPathGeomTest_testReadRectanglesAbsoluteClosed2;

static class TestDescription_SvgPathGeomTest_testReadRectanglesRelativeClosed2 : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadRectanglesRelativeClosed2() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 112, "testReadRectanglesRelativeClosed2" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadRectanglesRelativeClosed2(); }
} testDescription_SvgPathGeomTest_testReadRectanglesRelativeClosed2;

static class TestDescription_SvgPathGeomTest_testReadConcatenatedPaths : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadConcatenatedPaths() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 120, "testReadConcatenatedPaths" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadConcatenatedPaths(); }
} testDescription_SvgPathGeomTest_testReadConcatenatedPaths;

static class TestDescription_SvgPathGeomTest_testReadZeroLengthSubpaths : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadZeroLengthSubpaths() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 137, "testReadZeroLengthSubpaths" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadZeroLengthSubpaths(); }
} testDescription_SvgPathGeomTest_testReadZeroLengthSubpaths;

static class TestDescription_SvgPathGeomTest_testReadImplicitMoveto : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadImplicitMoveto() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 161, "testReadImplicitMoveto" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadImplicitMoveto(); }
} testDescription_SvgPathGeomTest_testReadImplicitMoveto;

static class TestDescription_SvgPathGeomTest_testReadFloatingPoint : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadFloatingPoint() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 182, "testReadFloatingPoint" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadFloatingPoint(); }
} testDescription_SvgPathGeomTest_testReadFloatingPoint;

static class TestDescription_SvgPathGeomTest_testReadImplicitSeparation : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadImplicitSeparation() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 208, "testReadImplicitSeparation" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadImplicitSeparation(); }
} testDescription_SvgPathGeomTest_testReadImplicitSeparation;

static class TestDescription_SvgPathGeomTest_testReadErrorMisplacedCharacter : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadErrorMisplacedCharacter() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 228, "testReadErrorMisplacedCharacter" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadErrorMisplacedCharacter(); }
} testDescription_SvgPathGeomTest_testReadErrorMisplacedCharacter;

static class TestDescription_SvgPathGeomTest_testReadErrorUnrecognizedCharacter : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadErrorUnrecognizedCharacter() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 257, "testReadErrorUnrecognizedCharacter" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadErrorUnrecognizedCharacter(); }
} testDescription_SvgPathGeomTest_testReadErrorUnrecognizedCharacter;

static class TestDescription_SvgPathGeomTest_testReadErrorTypo : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadErrorTypo() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 270, "testReadErrorTypo" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadErrorTypo(); }
} testDescription_SvgPathGeomTest_testReadErrorTypo;

static class TestDescription_SvgPathGeomTest_testReadErrorIllformedNumbers : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadErrorIllformedNumbers() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 284, "testReadErrorIllformedNumbers" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadErrorIllformedNumbers(); }
} testDescription_SvgPathGeomTest_testReadErrorIllformedNumbers;

static class TestDescription_SvgPathGeomTest_testReadErrorJunk : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadErrorJunk() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 317, "testReadErrorJunk" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadErrorJunk(); }
} testDescription_SvgPathGeomTest_testReadErrorJunk;

static class TestDescription_SvgPathGeomTest_testReadErrorStopReading : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testReadErrorStopReading() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 326, "testReadErrorStopReading" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testReadErrorStopReading(); }
} testDescription_SvgPathGeomTest_testReadErrorStopReading;

static class TestDescription_SvgPathGeomTest_testRoundTrip : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testRoundTrip() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 348, "testRoundTrip" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testRoundTrip(); }
} testDescription_SvgPathGeomTest_testRoundTrip;

static class TestDescription_SvgPathGeomTest_testMinexpPrecision : public CxxTest::RealTestDescription {
public:
 TestDescription_SvgPathGeomTest_testMinexpPrecision() : CxxTest::RealTestDescription( Tests_SvgPathGeomTest, suiteDescription_SvgPathGeomTest, 391, "testMinexpPrecision" ) {}
 void runTest() { if ( suite_SvgPathGeomTest ) suite_SvgPathGeomTest->testMinexpPrecision(); }
} testDescription_SvgPathGeomTest_testMinexpPrecision;

#include "../../src/xml/repr-action-test.h"

static XmlReprActionTest *suite_XmlReprActionTest = 0;

static CxxTest::List Tests_XmlReprActionTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<XmlReprActionTest> suiteDescription_XmlReprActionTest( "../../src/xml/repr-action-test.h", 11, "XmlReprActionTest", Tests_XmlReprActionTest, suite_XmlReprActionTest, 33, 34 );

static class TestDescription_XmlReprActionTest_testRollbackOfNodeAddition : public CxxTest::RealTestDescription {
public:
 TestDescription_XmlReprActionTest_testRollbackOfNodeAddition() : CxxTest::RealTestDescription( Tests_XmlReprActionTest, suiteDescription_XmlReprActionTest, 36, "testRollbackOfNodeAddition" ) {}
 void runTest() { if ( suite_XmlReprActionTest ) suite_XmlReprActionTest->testRollbackOfNodeAddition(); }
} testDescription_XmlReprActionTest_testRollbackOfNodeAddition;

static class TestDescription_XmlReprActionTest_testRollbackOfNodeRemoval : public CxxTest::RealTestDescription {
public:
 TestDescription_XmlReprActionTest_testRollbackOfNodeRemoval() : CxxTest::RealTestDescription( Tests_XmlReprActionTest, suiteDescription_XmlReprActionTest, 48, "testRollbackOfNodeRemoval" ) {}
 void runTest() { if ( suite_XmlReprActionTest ) suite_XmlReprActionTest->testRollbackOfNodeRemoval(); }
} testDescription_XmlReprActionTest_testRollbackOfNodeRemoval;

static class TestDescription_XmlReprActionTest_testRollbackOfNodeReordering : public CxxTest::RealTestDescription {
public:
 TestDescription_XmlReprActionTest_testRollbackOfNodeReordering() : CxxTest::RealTestDescription( Tests_XmlReprActionTest, suiteDescription_XmlReprActionTest, 64, "testRollbackOfNodeReordering" ) {}
 void runTest() { if ( suite_XmlReprActionTest ) suite_XmlReprActionTest->testRollbackOfNodeReordering(); }
} testDescription_XmlReprActionTest_testRollbackOfNodeReordering;

#include "../../src/xml/quote-test.h"

static XmlQuoteTest *suite_XmlQuoteTest = 0;

static CxxTest::List Tests_XmlQuoteTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<XmlQuoteTest> suiteDescription_XmlQuoteTest( "../../src/xml/quote-test.h", 15, "XmlQuoteTest", Tests_XmlQuoteTest, suite_XmlQuoteTest, 26, 27 );

static class TestDescription_XmlQuoteTest_testXmlQuotedStrlen : public CxxTest::RealTestDescription {
public:
 TestDescription_XmlQuoteTest_testXmlQuotedStrlen() : CxxTest::RealTestDescription( Tests_XmlQuoteTest, suiteDescription_XmlQuoteTest, 29, "testXmlQuotedStrlen" ) {}
 void runTest() { if ( suite_XmlQuoteTest ) suite_XmlQuoteTest->testXmlQuotedStrlen(); }
} testDescription_XmlQuoteTest_testXmlQuotedStrlen;

static class TestDescription_XmlQuoteTest_testXmlQuoteStrdup : public CxxTest::RealTestDescription {
public:
 TestDescription_XmlQuoteTest_testXmlQuoteStrdup() : CxxTest::RealTestDescription( Tests_XmlQuoteTest, suiteDescription_XmlQuoteTest, 50, "testXmlQuoteStrdup" ) {}
 void runTest() { if ( suite_XmlQuoteTest ) suite_XmlQuoteTest->testXmlQuoteStrdup(); }
} testDescription_XmlQuoteTest_testXmlQuoteStrdup;

#include "../../src/util/list-container-test.h"

static ListContainerTest *suite_ListContainerTest = 0;

static CxxTest::List Tests_ListContainerTest = { 0, 0 };
CxxTest::DynamicSuiteDescription<ListContainerTest> suiteDescription_ListContainerTest( "../../src/util/list-container-test.h", 28, "ListContainerTest", Tests_ListContainerTest, suite_ListContainerTest, 38, 39 );

static class TestDescription_ListContainerTest_testRangeConstructor : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testRangeConstructor() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 41, "testRangeConstructor" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testRangeConstructor(); }
} testDescription_ListContainerTest_testRangeConstructor;

static class TestDescription_ListContainerTest_testEqualityTests : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testEqualityTests() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 60, "testEqualityTests" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testEqualityTests(); }
} testDescription_ListContainerTest_testEqualityTests;

static class TestDescription_ListContainerTest_testLessThan : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testLessThan() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 79, "testLessThan" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testLessThan(); }
} testDescription_ListContainerTest_testLessThan;

static class TestDescription_ListContainerTest_testAssignmentOperator : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testAssignmentOperator() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 98, "testAssignmentOperator" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testAssignmentOperator(); }
} testDescription_ListContainerTest_testAssignmentOperator;

static class TestDescription_ListContainerTest_testFillConstructor : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testFillConstructor() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 110, "testFillConstructor" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testFillConstructor(); }
} testDescription_ListContainerTest_testFillConstructor;

static class TestDescription_ListContainerTest_testContainerSize : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testContainerSize() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 116, "testContainerSize" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testContainerSize(); }
} testDescription_ListContainerTest_testContainerSize;

static class TestDescription_ListContainerTest_testAppending : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testAppending() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 130, "testAppending" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testAppending(); }
} testDescription_ListContainerTest_testAppending;

static class TestDescription_ListContainerTest_testBulkAppending : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testBulkAppending() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 141, "testBulkAppending" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testBulkAppending(); }
} testDescription_ListContainerTest_testBulkAppending;

static class TestDescription_ListContainerTest_testPrepending : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testPrepending() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 151, "testPrepending" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testPrepending(); }
} testDescription_ListContainerTest_testPrepending;

static class TestDescription_ListContainerTest_testSingleValueInsertion : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testSingleValueInsertion() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 162, "testSingleValueInsertion" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testSingleValueInsertion(); }
} testDescription_ListContainerTest_testSingleValueInsertion;

static class TestDescription_ListContainerTest_testSingleValueErasure : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testSingleValueErasure() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 181, "testSingleValueErasure" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testSingleValueErasure(); }
} testDescription_ListContainerTest_testSingleValueErasure;

static class TestDescription_ListContainerTest_testPopFront : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testPopFront() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 203, "testPopFront" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testPopFront(); }
} testDescription_ListContainerTest_testPopFront;

static class TestDescription_ListContainerTest_testEraseAfter : public CxxTest::RealTestDescription {
public:
 TestDescription_ListContainerTest_testEraseAfter() : CxxTest::RealTestDescription( Tests_ListContainerTest, suiteDescription_ListContainerTest, 228, "testEraseAfter" ) {}
 void runTest() { if ( suite_ListContainerTest ) suite_ListContainerTest->testEraseAfter(); }
} testDescription_ListContainerTest_testEraseAfter;

#include <cxxtest/Root.cpp>

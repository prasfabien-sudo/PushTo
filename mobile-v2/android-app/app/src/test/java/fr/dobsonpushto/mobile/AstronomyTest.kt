package fr.dobsonpushto.mobile

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class AstronomyTest {
    @Test
    fun angularDifferenceUsesTheShortestSignedPathAcrossNorth() {
        assertEquals(2.0, Astronomy.angularDifference(1.0, 359.0), 1e-9)
        assertEquals(-2.0, Astronomy.angularDifference(359.0, 1.0), 1e-9)
    }

    @Test
    fun localSiderealTimeStaysWithinOneSiderealDay() {
        val siderealTime = Astronomy.localSiderealTime(
            longitude = -122.3,
            epochMillis = 1_704_067_200_000L
        )

        assertTrue(siderealTime >= 0.0)
        assertTrue(siderealTime < 24.0)
    }

    @Test
    fun equatorialCoordinatesConvertToFiniteHorizontalCoordinates() {
        val horizontal = Astronomy.toHorizontal(
            rightAscensionHours = 5.0,
            declinationDegrees = 20.0,
            latitudeDegrees = 46.0,
            longitudeDegrees = 3.0,
            epochMillis = 1_704_067_200_000L
        )

        assertTrue(horizontal.azimuth >= 0.0 && horizontal.azimuth < 360.0)
        assertTrue(horizontal.altitude.isFinite())
        assertTrue(horizontal.altitude in -90.0..91.0)
    }

    @Test
    fun visibilityReportsObjectsThatNeverRiseAboveTheHorizonThreshold() {
        val visibility = Astronomy.timeUntilVisible(
            raHours = 5.0,
            decDegrees = -20.0,
            latitude = 80.0,
            longitude = 0.0,
            epochMillis = 1_704_067_200_000L
        )

        assertEquals("Non visible cette nuit", visibility)
    }
}

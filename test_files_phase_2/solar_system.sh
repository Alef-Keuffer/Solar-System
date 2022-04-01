# https://www.enchantedlearning.com/subjects/astronomy/planets/
MERCURY_R=30
VENUS_R=$(python -c "print( $MERCURY_R * 1.5 )")
EARTH_R=$(python -c "print( $MERCURY_R * 1.55 )")
MARS_R=$(python -c "print( $MERCURY_R * 1.4 )")
JUPITER_R=$(python -c "print( $MERCURY_R * 2.8 )")
SATURN_R=$(python -c "print( $MERCURY_R * 2 )")
URANUS_R=$(python -c "print( $MERCURY_R * 1.9 )")
NEPTUNE_R=$(python -c "print( $MERCURY_R * 1.5 )")
SUN_R=$(python -c "print( $MERCURY_R * 3.3 )")

../bin/generator sphere $MERCURY_R 16 16 mercury.3d
../bin/generator sphere "$VENUS_R" 16 16 venus.3d
../bin/generator sphere "$EARTH_R" 16 16 earth.3d
../bin/generator sphere "$MARS_R" 16 16 mars.3d
../bin/generator sphere "$JUPITER_R" 16 16 jupiter.3d
../bin/generator sphere "$SATURN_R" 16 16 saturn.3d
../bin/generator sphere "$URANUS_R" 16 16 uranus.3d
../bin/generator sphere "$NEPTUNE_R" 16 16 neptune.3d
../bin/generator sphere "$SUN_R" 16 16 sun.3d

MERCURY_D=$(((SUN_R+MERCURY_R)*1.5))
VENUS_D=$(((MERCURY_D+VENUS_R)*1.5))
EARTH_D=$(((VENUS_D+EARTH_R)*1.5))
MARS_D=$(((EARTH_D+MARS_R)*1.5))
JUPITER_D=$(((MARS_D+JUPITER_R)*1.5))
SATURN_D=$(((JUPITER_D+SATURN_R)*1.5))
URANUS_D=$(((SATURN_D+URANUS_R)*1.5))
NEPTUNE_D=$(((URANUS_D+NEPTUNE_R)*1.5))

# shellcheck disable=SC2079
sed -e "s/MERCURY/$MERCURY_D/g"\
    -e "s/VENUS/$VENUS_D/g"\
    -e "s/EARTH/$EARTH_D/g"\
    -e "s/MARS/$MARS_D/g"\
    -e "s/JUPITER/$JUPITER_D/g"\
    -e "s/SATURN/$SATURN_D/g"\
    -e "s/URANUS/$URANUS_D/g"\
    -e "s/NEPTUNE/$NEPTUNE_D/g"\
    solar_system.template.xml > solar_system.xml

../bin/engine solar_system.xml
rm mercury.3d venus.3d earth.3d mars.3d jupiter.3d saturn.3d uranus.3d neptune.3d sun.3d solar_system.xml
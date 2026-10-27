struct vector3_t
{
        double x;
        double y;
        double z;
};

struct ecefPosition_t
{
        vector3_t coordinates; // Earth-fixed Cartesian coordinates, km
};

struct geographicPosition_t
{
        double latitude;  // degrees
        double longitude; // degrees
        double altitude;  // km above the reference ellipsoid
};

struct temeState_t
{
        vector3_t position; // km, TEME
        vector3_t velocity; // km/s, TEME
};

struct orbitalPlanePosition_t
{
        double xKm;
        double yKm;
};

struct satelliteState_t
{
        ecefPosition_t position;
        vector3_t velocity; // km/s, expressed in ECEF
};
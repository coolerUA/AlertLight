#ifndef REGIONMAPPER_H
#define REGIONMAPPER_H

#include <Arduino.h>

struct RegionInfo {
    uint16_t id;
    const char* name;
};

// Main Ukrainian regions (states)
const RegionInfo REGIONS[] = {
    {0, "Тестовий регіон"},
    {3, "Хмельницька область"},
    {4, "Вінницька область"},
    {5, "Рівненська область"},
    {8, "Волинська область"},
    {9, "Дніпропетровська область"},
    {10, "Житомирська область"},
    {11, "Закарпатська область"},
    {12, "Запорізька область"},
    {13, "Івано-Франківська область"},
    {14, "Київська область"},
    {15, "Кіровоградська область"},
    {16, "Луганська область"},
    {17, "Миколаївська область"},
    {18, "Одеська область"},
    {19, "Полтавська область"},
    {20, "Сумська область"},
    {21, "Тернопільська область"},
    {22, "Харківська область"},
    {23, "Херсонська область"},
    {24, "Черкаська область"},
    {25, "Чернігівська область"},
    {26, "Чернівецька область"},
    {27, "Львівська область"},
    {28, "Донецька область"},
    {31, "м. Київ"},
    {564, "м. Запоріжжя"},
    {1293, "м. Харків"},
    {9999, "АР Крим"}
};

const int REGIONS_COUNT = sizeof(REGIONS) / sizeof(RegionInfo);

class RegionMapper {
public:
    // Get region name by ID, returns "Unknown Region" if not found
    static String getRegionName(uint16_t regionId) {
        for (int i = 0; i < REGIONS_COUNT; i++) {
            if (REGIONS[i].id == regionId) {
                String name = String(REGIONS[i].name);
                // Trim common suffixes to fit on display
                name.replace(" область", "");
                name.replace(" територіальна громада", "");
                name.replace("Автономна Республіка ", "");
                return name;
            }
        }
        return "Region " + String(regionId);
    }

    // Get region ID by name, returns 0 if not found
    static uint16_t getRegionId(const String& regionName) {
        for (int i = 0; i < REGIONS_COUNT; i++) {
            if (String(REGIONS[i].name) == regionName) {
                return REGIONS[i].id;
            }
        }
        return 0;
    }

    // Get total count of regions
    static int getRegionCount() {
        return REGIONS_COUNT;
    }

    // Get region info by index
    static const RegionInfo& getRegion(int index) {
        return REGIONS[index];
    }
};

#endif // REGIONMAPPER_H

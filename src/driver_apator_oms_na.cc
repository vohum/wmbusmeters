/*
 Copyright (C) 2017-2022 Fredrik Öhrström (gpl-3.0-or-later)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include"meters_common_implementation.h"

namespace
{
    struct Driver : public virtual MeterCommonImplementation {
        Driver(MeterInfo &mi, DriverInfo &di);
    };

    static bool ok = registerDriver([](DriverInfo&di)
    {
        di.setName("apator_oms_na");
        di.setMeterType(MeterType::WaterMeter);
        di.addLinkMode(LinkMode::T1);
        di.addDetection(MANUFACTURER_APA,  0x07,  0x0C);
        di.setConstructor([](MeterInfo& mi, DriverInfo& di){ return shared_ptr<Meter>(new Driver(mi, di)); });
    });

    Driver::Driver(MeterInfo &mi, DriverInfo &di) : MeterCommonImplementation(mi, di)
    {
        addNumericFieldWithExtractor(
            "total",
            "The total water consumption recorded by this meter.",
            PrintProperty::JSON | PrintProperty::FIELD | PrintProperty::IMPORTANT,
            Quantity::Volume,
            VifScaling::Auto,
            FieldMatcher::build()
            .set(MeasurementType::Instantaneous)
            .set(VIFRange::Volume)
            );

        addNumericFieldWithExtractor(
            "target",
             "The total water consumption recorded at the beginning of this month.",
            PrintProperty::JSON | PrintProperty::FIELD,
            Quantity::Volume,
            VifScaling::Auto,
            FieldMatcher::build()
            .set(MeasurementType::Instantaneous)
            .set(VIFRange::Volume)
            .set(StorageNr(1))
            );

        addStringFieldWithExtractorAndLookup(
            "current_status",
            "Status and error flags.",
            PrintProperty::JSON | PrintProperty::FIELD,
            FieldMatcher::build()
            .set(DifVifKey("03FD17")),
            {
                {
                    {
                        "ERROR_FLAGS",
                        Translate::Type::BitToString,
                        0xffffff,
                        "OK",
                        {
                            /* What are the bits?
                               According to the manual this meter offers these alarms:
                               Back flow
                               Meter leak
                               Water main leak
                               Zero flow
                               Tampering detected
                               No water
                               Low battery
                            */
                        }
                    },
                },
            });

        addNumericFieldWithExtractor(
            "total_backward_flow",
            "The total backward water volume recorded by this meter.",
            PrintProperty::JSON | PrintProperty::FIELD,
            Quantity::Volume,
            VifScaling::Auto,
            FieldMatcher::build()
            .set(DifVifKey("04933C"))
            );
    }
}
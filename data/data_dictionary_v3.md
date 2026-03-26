# Data Dictionary

Brazil Environment–Health Multimodal Dataset

This dataset integrates environmental, health, climate, infrastructure, and socioeconomic indicators for Brazilian federative units between 2015 and 2023.
The dataset was designed to support multisensory data visualization and environmental health analysis.

| Column                       | Type        | Description                                | Unit            | Source              | Derived From                 | Classification Rule / Notes                                    |
| ---------------------------- | ----------- | ------------------------------------------ | --------------- | ------------------- | ---------------------------- | -------------------------------------------------------------- |
| id                           | integer     | Unique observation identifier              | —               | Generated           | —                            | State–year record                                              |
| year                         | integer     | Observation year                           | year            | IBGE                | —                            | Temporal dimension                                             |
| state                        | categorical | Brazilian state name                       | —               | IBGE                | —                            |                                                                |
| state_code                   | categorical | Two-letter state abbreviation              | —               | IBGE                | —                            |                                                                |
| region                       | categorical | Brazilian geographic region                | —               | IBGE                | —                            | North, Northeast, Midwest, Southeast, South                    |
| pm25_mean                    | numeric     | Annual mean PM2.5 concentration            | µg/m³           | OpenAQ / Copernicus | —                            | Fine particulate matter                                        |
| pm10_mean                    | numeric     | Annual mean PM10 concentration             | µg/m³           | OpenAQ / Copernicus | —                            | Coarse particles                                               |
| no2_mean                     | numeric     | Annual mean nitrogen dioxide concentration | µg/m³           | OpenAQ              | —                            | Traffic pollution indicator                                    |
| o3_mean                      | numeric     | Annual mean ozone concentration            | µg/m³           | Copernicus          | —                            | Secondary pollutant                                            |
| wildfire_hotspots            | numeric     | Annual wildfire hotspot count              | count           | INPE                | —                            | Biomass burning indicator                                      |
| deforestation_rate           | numeric     | Annual deforested area                     | km²             | INPE PRODES         | —                            | Environmental pressure                                         |
| respiratory_hospitalizations | numeric     | Respiratory hospitalizations               | cases/100k      | DATASUS             | —                            | Respiratory diseases                                           |
| asthma_rate                  | numeric     | Asthma hospitalization rate                | cases/100k      | DATASUS             | —                            |                                                                |
| copd_rate                    | numeric     | COPD hospitalization rate                  | cases/100k      | DATASUS             | —                            |                                                                |
| respiratory_mortality        | numeric     | Mortality from respiratory diseases        | deaths/100k     | DATASUS             | —                            |                                                                |
| sanitation_coverage          | numeric     | Population with sanitation access          | %               | SNIS / IBGE         | —                            | Environmental health factor                                    |
| primary_care_coverage        | numeric     | Primary healthcare coverage                | %               | Ministry of Health  | —                            |                                                                |
| hospital_beds_per1000        | numeric     | Hospital beds per 1000 inhabitants         | beds/1000       | Ministry of Health  | —                            | Healthcare capacity                                            |
| population_density           | numeric     | Population density                         | inhabitants/km² | IBGE                | —                            | Exposure indicator                                             |
| gdp_per_capita               | numeric     | GDP per capita                             | BRL             | IBGE                | —                            | Economic indicator                                             |
| annual_mean_temperature      | numeric     | Mean annual temperature                    | °C              | INMET               | —                            | Climate condition                                              |
| relative_humidity            | numeric     | Mean annual relative humidity              | %               | INMET               | —                            | Air dispersion factor                                          |
| urbanization_rate            | numeric     | Population living in urban areas           | %               | IBGE                | —                            | Urban exposure indicator                                       |
| pm25_category                | categorical | PM2.5 air quality category                 | —               | Derived             | pm25_mean                    | WHO Air Quality thresholds                                     |
| pm10_category                | categorical | PM10 air quality category                  | —               | Derived             | pm10_mean                    | WHO thresholds                                                 |
| no2_pollution_level          | categorical | NO₂ pollution level                        | —               | Derived             | no2_mean                     | WHO thresholds                                                 |
| o3_pollution_level           | categorical | O₃ pollution level                         | —               | Derived             | o3_mean                      | WHO thresholds                                                 |
| wildfire_intensity           | categorical | Wildfire intensity level                   | —               | Derived             | wildfire_hotspots            | Quartile / empirical classification                            |
| deforestation_level          | categorical | Deforestation pressure level               | —               | Derived             | deforestation_rate           | Threshold classification                                       |
| respiratory_risk_level       | categorical | Respiratory health risk level              | —               | Derived             | respiratory_hospitalizations | Quartile classification                                        |
| sanitation_level             | categorical | Sanitation infrastructure level            | —               | Derived             | sanitation_coverage          | Universal ≥90%, Adequate 70–90, Limited 50–70, Critical <50    |
| healthcare_capacity_level    | categorical | Healthcare infrastructure capacity         | —               | Derived             | hospital_beds_per1000        | Low <2, Moderate 2–3, High >3                                  |
| population_density_class     | categorical | Population density category                | —               | Derived             | population_density           | Rural <20, Semi-urban 20–100, Urban 100–300, Metropolitan >300 |
| economic_level               | categorical | Economic development level                 | —               | Derived             | gdp_per_capita               | World Bank income thresholds                                   |
| temperature_zone             | categorical | Climate temperature category               | —               | Derived             | annual_mean_temperature      | Mild <20, Warm 20–24, Hot 24–27, Very Hot >27                  |
| humidity_level               | categorical | Relative humidity class                    | —               | Derived             | relative_humidity            | Dry <50, Moderate 50–70, Humid 70–85, Very Humid >85           |
| urbanization_class           | categorical | Urbanization level                         | —               | Derived             | urbanization_rate            | Low <60, Medium 60–75, High 75–90, Highly urbanized >90        |



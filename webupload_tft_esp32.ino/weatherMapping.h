/*

Defined here:  https://www.metoffice.gov.uk/services/data/datapoint/code-definitions

Value	Description
NA	Not available
0	Clear night
1	Sunny day
2	Partly cloudy (night)
3	Partly cloudy (day)
4	Not used
5	Mist
6	Fog
7	Cloudy
8	Overcast
9	Light rain shower (night)
10	Light rain shower (day)
11	Drizzle
12	Light rain
13	Heavy rain shower (night)
14	Heavy rain shower (day)
15	Heavy rain
16	Sleet shower (night)
17	Sleet shower (day)
18	Sleet
19	Hail shower (night)
20	Hail shower (day)
21	Hail
22	Light snow shower (night)
23	Light snow shower (day)
24	Light snow
25	Heavy snow shower (night)
26	Heavy snow shower (day)
27	Heavy snow
28	Thunder shower (night)
29	Thunder shower (day)
30	Thunder

*/


// These link to the picture ID in Nextion editor
// These are the large icons

const int WEATHER_CODES_LARGE[] = {
72,  // Clear night  - 0
58,  // Sunny day - 1
63,  // Partly cloudy night - 2
48,  // Partly cloudy day 
48, // Not used, so placeholder
62,  // Mist
62, // Fog
47, // Cloudy
47, // Overcast
69, // Light rain night. Nice.
56, // Light rain day - 10
56, // Drizzle
56, // Light rain
65, // Heavy rain night
52, // Heavy rain day
52, // Heavy rain - 15
67, // Sleet night
54, // Sleet day
54, // Sleet
64, // Hail night
50, // Hail day - 20
50, // Hail
68, // Light snow night
55, // Light snow day
55, // Light snow
68, // Heavy snow night - 25
55, // Heavy snow day
55, // Heavy snow
70, // Thunder night
57, // Thunder shower day - 29
51, // Thunder day - 30
};

const int WEATHER_CODES_LARGE_NIGHT[] = {
72, // Clear night  - 0
58, // Sunny day - 1
63, // Partly cloudy night - 2
48, // Partly cloudy day 
48, // Not used, so placeholder
62, // Mist
62, // Fog
47, // Cloudy
47, // Overcast
69, // Light rain night. Nice.
56, // Light rain day - 10
69, // Drizzle
69, // Light rain
65, // Heavy rain night
52, // Heavy rain day
65, // Heavy rain - 15
67, // Sleet night
54, // Sleet day
67, // Sleet
64, // Hail night
50, // Hail day - 20
64, // Hail
68, // Light snow night
55, // Light snow day
68, // Light snow
68, // Heavy snow night - 25
55, // Heavy snow day
68, // Heavy snow
70, // Thunder night
57, // Thunder shower day - 29
51, // Thunder day - 30
};




const int WEATHER_CODES_SMALL[] = {
44, // 0	Clear night
30, // 1	Sunny day
35, // 2	Partly cloudy (night)
31, // 3	Partly cloudy (day)
31,  // 4	Not used
34, // 5	Mist
34, // 6	Fog
45, // 7	Cloudy.
19, // 8	Overcast
41, // 9	Light rain shower (night)
28, // 10	Light rain shower (day)
28, // 11	Drizzle
28, // 12	Light rain
37, // 13	Heavy rain shower (night)
24, // 14	Heavy rain shower (day)
24, // 15	Heavy rain
39, // 16	Sleet shower (night)
26, // 17	Sleet shower (day)
26, // 18	Sleet
36, // 19	Hail shower (night)
22, // 20	Hail shower (day)
22, // 21	Hail
40, // 22	Light snow shower (night)
27, // 23	Light snow shower (day)
27, // 24	Light snow
40, // 25	Heavy snow shower (night)
27, // 26	Heavy snow shower (day)
27, // 27	Heavy snow
42, // 28	Thunder shower (night)
29, // 29	Thunder shower (day)
29 // 30	Thunder
};

const int WEATHER_CODES_SMALL_NIGHT[] = {
44, // 0	Clear night
30, // 1	Sunny day
35, // 2	Partly cloudy (night)
31, // 3	Partly cloudy (day)
31,  // 4	Not used
34, // 5	Mist
34, // 6	Fog
35, // 7	Cloudy.
35, // 8	Overcast
41, // 9	Light rain shower (night)
28, // 10	Light rain shower (day)
41, // 11	Drizzle
41, // 12	Light rain
37, // 13	Heavy rain shower (night)
24, // 14	Heavy rain shower (day)
37, // 15	Heavy rain
39, // 16	Sleet shower (night)
26, // 17	Sleet shower (day)
39, // 18	Sleet
36, // 19	Hail shower (night)
22, // 20	Hail shower (day)
36, // 21	Hail
40, // 22	Light snow shower (night)
27, // 23	Light snow shower (day)
40, // 24	Light snow
40, // 25	Heavy snow shower (night)
27, // 26	Heavy snow shower (day)
40, // 27	Heavy snow
42, // 28	Thunder shower (night)
29, // 29	Thunder shower (day)
42 // 30	Thunder
};

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

const byte WEATHER_CODES_LARGE[] = {
3,  // Clear night  - 0
2,  // Sunny day - 1
31,  // Partly cloudy night - 2
8,  // Partly cloudy day 
43, // Not used, so placeholder
5,  // Mist
13, // Fog
25, // Cloudy
14, // Overcast
34, // Light rain night
17, // Light rain day - 10
17, // Drizzle
17, // Light rain
35, // Heavy rain night
18, // Heavy rain day
18, // Heavy rain - 15
40, // Sleet night
24, // Sleet day
24, // Sleet
40, // Hail night
24, // Hail day - 20
24, // Hail
38, // Light snow night
21, // Light snow day
22, // Light snow
39, // Heavy snow night - 25
39, // Heavy snow day
23, // Heavy snow
33, // Thunder night
27, // Thunder shower day - 29
27, // Thunder day - 30
};

const byte WEATHER_CODES_LARGE_NIGHT[] = {
3,  // Clear night  - 0
2,  // Sunny day - 1
31,  // Partly cloudy night - 2
8,  // Partly cloudy day 
43, // Not used, so placeholder
5,  // Mist
13, // Fog
32, // Cloudy
14, // Overcast
34, // Light rain night
17, // Light rain day - 10
17, // Drizzle
17, // Light rain
35, // Heavy rain night
18, // Heavy rain day
18, // Heavy rain - 15
40, // Sleet night
24, // Sleet day
24, // Sleet
40, // Hail night
24, // Hail day - 20
24, // Hail
38, // Light snow night
21, // Light snow day
22, // Light snow
39, // Heavy snow night - 25
39, // Heavy snow day
23, // Heavy snow
33, // Thunder night
27, // Thunder shower day - 29
27, // Thunder day - 30
};




const byte WEATHER_CODES_SMALL[] = {
47, // 0	Clear night
46, // 1	Sunny day
75, // 2	Partly cloudy (night)
52, // 3	Partly cloudy (day)
0,  // 4	Not used
49, // 5	Mist
57, // 6	Fog
69, // 7	Cloudy. Nice
58, // 8	Overcast
78, // 9	Light rain shower (night)
61, // 10	Light rain shower (day)
61, // 11	Drizzle
61, // 12	Light rain
79, // 13	Heavy rain shower (night)
62, // 14	Heavy rain shower (day)
62, // 15	Heavy rain
84, // 16	Sleet shower (night)
68, // 17	Sleet shower (day)
68, // 18	Sleet
84, // 19	Hail shower (night)
68, // 20	Hail shower (day)
68, // 21	Hail
82, // 22	Light snow shower (night)
66, // 23	Light snow shower (day)
66, // 24	Light snow
83, // 25	Heavy snow shower (night)
67, // 26	Heavy snow shower (day)
67, // 27	Heavy snow
77, // 28	Thunder shower (night)
71, // 29	Thunder shower (day)
71 // 30	Thunder
};

const byte WEATHER_CODES_SMALL_NIGHT[] = {
47, // 0	Clear night
46, // 1	Sunny day
75, // 2	Partly cloudy (night)
52, // 3	Partly cloudy (day)
0,  // 4	Not used
49, // 5	Mist
57, // 6	Fog
76, // 7	Cloudy.
58, // 8	Overcast
78, // 9	Light rain shower (night)
61, // 10	Light rain shower (day)
61, // 11	Drizzle
61, // 12	Light rain
79, // 13	Heavy rain shower (night)
62, // 14	Heavy rain shower (day)
62, // 15	Heavy rain
84, // 16	Sleet shower (night)
68, // 17	Sleet shower (day)
68, // 18	Sleet
84, // 19	Hail shower (night)
68, // 20	Hail shower (day)
68, // 21	Hail
82, // 22	Light snow shower (night)
66, // 23	Light snow shower (day)
66, // 24	Light snow
83, // 25	Heavy snow shower (night)
67, // 26	Heavy snow shower (day)
67, // 27	Heavy snow
77, // 28	Thunder shower (night)
71, // 29	Thunder shower (day)
71 // 30	Thunder
};

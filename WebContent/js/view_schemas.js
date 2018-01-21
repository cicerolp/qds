var view_schemas = {
    "example.nds": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "debug",
        title: "Example CSV",
        tile: [{
            title: "Location",
            value: "5",
            color: "#ffffff"
        },],
        views: [
            {
                type: "histogram",
                title: "Gênero",
                on_menu: true,
                div: "#top-section",
                size: 25,
                field: {
                    name: "c_0",
                    values: ["M", "F"]
                }
            },
            {
                type: "histogram",
                title: "Idade",
                on_menu: true,
                div: "#top-section",
                size: 25,
                field: {
                    name: "c_1",
                    values: ["0-20", "21-30", "31-40", "41-50", "51-60", "61-70"]
                }
            },
            {
                type: "histogram",
                title: "Status",
                on_menu: true,
                div: "#top-section",
                size: 25,
                field: {
                    name: "c_2",
                    values: ["Off", "On"]
                }
            },
            {
                type: "histogram",
                title: "Origem",
                on_menu: true,
                div: "#top-section",
                size: 25,
                field: {
                    name: "c_3",
                    values: ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
                }
            },
            {
                type: "time-series",
                title: "Timeline",
                on_menu: false,
                div: "#section",
                size: 98,
                field: {
                    name: "t_4",
                }
            },
        ]
    },

    "brightkite": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "ryw",
        PLOTTING_TRANSFORM: "density_scaling",
        title: "Brightkite Checkins",
        tile: [{
            title: "Location",
            value: "0",
            color: "#ffffff"
        },],
        views: [
            {
                type: "histogram",
                title: "Day of Week",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "c_1",
                    values: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
                    ]
                }
            },
            {
                type: "time-series",
                title: "Timeline",
                on_menu: false,
                div: "#section",
                size: 50,
                field: {
                    name: "t_3",
                }
            },
            {
                type: "histogram",
                title: "Hour Of Day",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "c_2",
                    values: ["00", "01", "02", "03", "04", "05", "06",
                        "07", "08", "09", "10", "11", "12", "13", "14",
                        "15", "16", "17", "18", "19", "20", "21", "22",
                        "23"
                    ]
                }
            }
        ]
    },

    "twitter-small": {
        PLOTTING: "black",
        PLOTTING_MODE: "circle",
        PLOTTING_COLOR_SCALE: "ryw",
        PLOTTING_TRANSFORM: "density_scaling",
        title: "Twitter (small)",
        tile: [{
            title: "Location",
            value: "0",
            color: "#0000ff"
        },],
        views: [
            {
                type: "histogram",
                title: "Device",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "c_1",
                    values: ["None", "iPhone", "Android", "iPad", "Windows"]
                }
            },
            {
                type: "time-series",
                title: "Timeline",
                on_menu: true,
                div: "#section",
                size: 75,
                field: {
                    name: "t_2",
                }
            },
        ]
    },

    "performance": {
        PLOTTING: "black",
        PLOTTING_MODE: "circle",
        PLOTTING_COLOR_SCALE: "bbb",
        PLOTTING_TRANSFORM: "density_scaling",
        title: "Airline On-Time Statistics",
        tile: [{
            title: "Origin",
            value: "0",
            color: "#0000ff"
        }],
        views: [
            {
                type: "histogram",
                title: "Delay",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "c_1",
                    values: ["61+min early", "31-60min early",
                        "16-30min early", "6-15min early",
                        "5min early/late", "6-15min late",
                        "16-30min late", "31-60min late", "61+min late"
                    ]
                }
            },
            {
                type: "time-series",
                title: "Timeline",
                on_menu: true,
                div: "#section",
                size: 50,
                field: {
                    name: "t_3",
                }
            },
            {
                type: "histogram",
                title: "Carrier",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_2",
                    values: ["PS", "TW", "UA", "WN", "EA", "HP", "NW",
                        "PA (1)", "PI", "CO", "DL", "AA", "US", "AS",
                        "ML (1)", "AQ", "MQ", "OO", "XE", "TZ", "EV",
                        "FL", "B6", "DH", "HA", "OH", "F9", "YV", "9E"
                    ]

                }
            }
        ],
    },

    "gowalla": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "ryw",
        PLOTTING_TRANSFORM: "density_scaling",
        title: "Gowalla Checkins",
        tile: [{
            title: "Location",
            value: "0",
            color: "#ff7800"
        },],
        views: [
            {
                type: "histogram",
                title: "Day of Week",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "c_1",
                    values: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
                        "Sun"
                    ]
                }
            },
            {
                type: "histogram",
                title: "Hour Of Day",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "c_2",
                    values: ["00", "01", "02", "03", "04", "05", "06",
                        "07", "08", "09", "10", "11", "12", "13", "14",
                        "15", "16", "17", "18", "19", "20", "21", "22",
                        "23"
                    ]
                }
            },
            {
                type: "time-series",
                title: "Timeline",
                on_menu: true,
                div: "#section",
                size: 50,
                field: {
                    name: "t_3",
                }
            },
            {
                type: "binned-scatterplot",
                title: "Day of Week x Hour of Day",
                on_menu: false,
                div: "#top-section",
                size: 25,
                field: {
                    name: "binned"
                },
                field_x: {
                    name: "c_2",
                    title: "Hour of Day",
                    values: ["00", "01", "02", "03", "04", "05", "06",
                        "07", "08", "09", "10", "11", "12", "13", "14",
                        "15", "16", "17", "18", "19", "20", "21", "22",
                        "23"
                    ]
                },
                field_y: {
                    name: "c_1",
                    title: "Day of Week",
                    values: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
                        "Sun"
                    ]
                }
            }
        ]
    }
};
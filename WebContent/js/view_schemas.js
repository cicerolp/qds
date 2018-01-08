var view_schemas = {

    "file2.nds": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "ryw",
        title: "Tabela Contatos",
        tile: [{
            title: "Location",
            value: "15",
            color: "#ffffff"
        }, ],
        views: [{
                type: "histogram",
                title: "Gender",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_0",
                    values: ["Male", "Female"]
                }
            },
            {
                type: "histogram",
                title: "Permission Status",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_3",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "Deliverability Status",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_4",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "Phone",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_7",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "Address",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_9",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "Age",
                on_menu: true,
                div: "#top-section",
                size: 15,
                field: {
                    name: "c_12",
                    values: ["0-20", "21-30", "31-40", "41-50", "51-60", "61+"]
                }
            },
            {
                type: "histogram",
                title: "Mobile",
                on_menu: true,
                div: "#top-section",
                size: 15,
                field: {
                    name: "c_10",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "CPF",
                on_menu: true,
                div: "#top-section",
                size: 15,
                field: {
                    name: "c_11",
                    values: ["On", "Off"]
                }
            },
            
            {
                type: "time-series",
                title: "Timeline",
                on_menu: true,
                div: "#top-section",
                size: 35,
                field: {
                    name: "t_14",
                }
            }
        ]
    },"file.nds": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "ryw",
        title: "Tabela Contatos",
        tile: [{
            title: "Location",
            value: "15",
            color: "#ffffff"
        }, ],
        views: [{
                type: "histogram",
                title: "Gender",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_0",
                    values: ["Male", "Female"]
                }
            },
            {
                type: "histogram",
                title: "Permission Status",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_3",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "Deliverability Status",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_4",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "Phone",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_7",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "Address",
                on_menu: true,
                div: "#section",
                size: 20,
                field: {
                    name: "c_9",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "Age",
                on_menu: true,
                div: "#top-section",
                size: 15,
                field: {
                    name: "c_12",
                    values: ["0-20", "21-30", "31-40", "41-50", "51-60", "61+"]
                }
            },
            {
                type: "histogram",
                title: "Mobile",
                on_menu: true,
                div: "#top-section",
                size: 15,
                field: {
                    name: "c_10",
                    values: ["On", "Off"]
                }
            },
            {
                type: "histogram",
                title: "CPF",
                on_menu: true,
                div: "#top-section",
                size: 15,
                field: {
                    name: "c_11",
                    values: ["On", "Off"]
                }
            },
            
            {
                type: "time-series",
                title: "Timeline",
                on_menu: true,
                div: "#top-section",
                size: 35,
                field: {
                    name: "t_14",
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
        }, ],
        views: [{
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
        views: [{
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

    "brightkite": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "ryw",
        title: "Brightkite Checkins",
        tile: [{
            title: "Location",
            value: "0",
            color: "#ffffff"
        }, ],
        views: [{
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
        }, ],
        views: [{
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
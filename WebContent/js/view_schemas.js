var view_schemas = {
     "brightkite": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "bbb",
        PLOTTING_TRANSFORM: "density_scaling",
        title: "Brightkite Checkins",
        tile: [{
            title: "Location",
            value: "coord",
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
                    name: "day_of_week",
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
                    name: "date",
                }
            },
            {
                type: "histogram",
                title: "Hour Of Day",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "hour_of_day",
                    values: ["00", "01", "02", "03", "04", "05", "06",
                        "07", "08", "09", "10", "11", "12", "13", "14",
                        "15", "16", "17", "18", "19", "20", "21", "22",
                        "23"
                    ]
                }
            }
        ]
    },

    "green_tripdata": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "bbb",
        PLOTTING_TRANSFORM: "density_scaling",
        title: "Green Trip Data",
        tile: [{
            title: "Pickup Location",
            value: "pickup",
            color: "#ffffff"
        },],
        views: [
            {
                type: "histogram",
                title: "Passenger Count",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "passenger_count",
                    values: ["1", "2", "3", "4+"]
                }
            },
            {
                type: "time-series",
                title: "Pickup Datetime",
                on_menu: false,
                div: "#section",
                size: 50,
                field: {
                    name: "pickup_datetime",
                }
            },
            {
                type: "histogram",
                title: "Payment Type",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "payment_type",
                    values: ["Credit", "Cash", "No Charge", "Dispute", "Voided Trip", "Unknown"]
                }
            }
        ]
    },

    "yellow_tripdata": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "bbb",
        PLOTTING_TRANSFORM: "density_scaling",
        title: "Yellow Trip Data",
        tile: [{
            title: "Pickup Location",
            value: "pickup",
            color: "#ffffff"
        },],
        views: [
            {
                type: "histogram",
                title: "Passenger Count",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "passenger_count",
                    values: ["1", "2", "3", "4+"]
                }
            },
            {
                type: "time-series",
                title: "Pickup Datetime",
                on_menu: false,
                div: "#section",
                size: 50,
                field: {
                    name: "pickup_datetime",
                }
            },
            {
                type: "histogram",
                title: "Payment Type",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "payment_type",
                    values: ["Credit", "Cash", "No Charge", "Dispute", "Voided Trip", "Unknown"]
                }
            }
        ]
    },

    "on_time_performance": {
        PLOTTING: "black",
        PLOTTING_MODE: "rect",
        PLOTTING_COLOR_SCALE: "ryw",
        PLOTTING_TRANSFORM: "density_scaling",
        title: "Trip Data",
        tile: [{
            title: "Pickup Location",
            value: "origin_airport",
            color: "#ffffff"
        },],
        views: [
            /*{
                type: "histogram",
                title: "Cancelled",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "cancelled",
                    values: ["No", "Yes"]
                }
            },*/
            {
                type: "time-series",
                title: "CRS Dep Time",
                on_menu: false,
                div: "#section",
                size: 100,
                field: {
                    name: "crs_dep_time",
                }
            },
            /*{
                type: "histogram",
                title: "Diverted",
                on_menu: true,
                div: "#section",
                size: 25,
                field: {
                    name: "diverted",
                    values: ["No", "Yes"]
                }
            },*/
            /*{
                type: "histogram",
                title: "Unique Carrier",
                on_menu: true,
                div: "#top-section",
                size: 100,
                field: {
                    name: "unique_carrier",
                    values: ["02Q","04Q","05Q","06Q","07Q","09Q","0BQ","0CQ","0FQ","0GQ","0HQ","0J","0JQ","0LQ","0MQ","0OQ","0Q","0QQ","0RQ","0TQ","0UQ","0VQ","0WQ","0YQ","10Q","12Q","13Q","14Q","15Q","17Q","1AQ","1BQ","1DQ","1EQ","1FQ","1HQ","1I","1IQ","1JQ","1LQ","1MQ","1NQ","1PQ","1QQ","1RQ","1SQ","1TQ","1UQ","1WQ","1XQ","1YQ","1ZQ","20Q","22Q","23Q","24Q","25Q","26Q","27Q","28Q","29Q","2AQ","2BQ","2E","2EQ","2F","2GQ","2HQ","2JQ","2K","2KQ","2LQ","2M","2NQ","2O","2OQ","2P","2PQ","2R","2RQ","2T","2TQ","2U","2UQ","30Q","34Q","35Q","36Q","37","37Q","38Q","39Q","3AQ","3C","3F","3M","3S","3SD","3U","4B","4B (1)","4C","4E","4H","4M","4M (1)","4N","4O","4R","4S","4S (1)","4T","4W","4Y","5B","5C","5D","5F","5G","5G (1)","5J","5J (1)","5L","5V","5X","5Y","6A","6B","6C","6F","6H","6P","6R","6U","6Y","7B","7C","7F","7G","7G (1)","7H","7I","7L","7M","7N","7P","7Q","7S","7S (1)","7Z","8C","8D","8E","8F","8I","8J","8L","8N","8Q","8R","8V","9E","9J","9K","9L","9M","9N","9R","9S","9T","9V","9W","A0","A2","A4","A7","A7 (1)","AA","AAA","AAB","AAB (1)","AAE","AAI","AAL","AAP","AAQ","AAR","AAT","AAV","AAZ","AB","ABH","ABI","ABO","ABX","AC","ACA","ACD","ACG","ACH","ACI","ACJ","ACK","ACL","ACN","ACO","ACP","ACQ","ACS","ACT","ACW","AD","AD (1)","ADB","ADQ","ADR","ADV","AE","AEC","AED","AEF","AEO","AEQ","AEX","AF","AFA","AFE","AFG","AFQ","AFS","AGA","AGQ","AHA","AHC","AHQ","AI","AIA","AIC","AIE","AIG","AIK","AIM","AIN","AIQ","AIT","AIY","AJ","AJQ","AKQ","AL","ALD","ALE","ALI","ALK","ALL","ALQ","ALS","ALT","ALU","ALX","AM","AMA","AME","AMI","AMQ","AMR","AMT","ANB","ANC","AND","ANI","ANM","ANO","ANT","AO","AOI","AON","AOQ","AOR","APE","APH","API","APL","APN","APQ","APS","APY","AQ","AQQ","AR","ARA","ARB","ARD","ARI","ARM","ARN","ARO","ARP","ARQ","ARR","ARS","ARZ","AS","ASC","ASD","ASH","ASI","ASN","ASP","ASQ","ASR","AST","ASU","ASW","AT","ATH","ATN","ATO","ATQ","ATS","ATT","ATX","AUX","AV","AVA","AVC","AVG","AVI","AVL","AVQ","AVR","AVS","AVZ","AWA","AWQ","AX","AXQ","AY","AYQ","AZ","AZP","AZQ","B0","B6","BA","BAA","BAC","BAH","BAK","BAN","BAQ","BAR","BAT","BAY","BBQ","BC","BCA","BCQ","BD","BDE","BDQ","BE","BEA","BEN","BEQ","BES","BEY","BF","BFG","BFQ","BG","BGQ","BHK","BHO","BHQ","BIA","BIQ","BIR","BJQ","BLQ","BM","BMT","BN","BN (1)","BNQ","BOA","BOI","BOQ","BQ","BR","BR (1)","BRK","BRO","BRW","BTQ","BUH","BUQ","BUQ (1)","BUR","BW","BX","BXQ","BY","BYA","BZQ","C5","C6","C8","C8 (1)","CA","CAC","CAF","CAI","CAM","CAP","CAQ","CAT","CAV","CAZ","CB","CBA","CC","CC (1)","CCA","CCO","CDQ","CEN","CEQ","CET","CF","CGA","CGL","CGO","CH","CHD","CHE","CHF","CHI","CHL","CHO","CHS","CHT","CI","CIQ","CIS","CJ","CK","CKI","CL","CLB","CLD","CLF","CLQ","CM","CMB","CMD","CMQ","CMR","CNN","CO","COB","COC","COK","COL","COM","COR","COS","COT","COU","CP","CP (1)","CPA","CPC","CPQ","CPT","CQI","CRA","CRB","CRE","CRH","CRN","CRO","CRP","CRT","CRV","CS","CSA","CSK","CSL","CSM","CSN","CSQ","CTA","CTH","CTL","CTQ","CTR","CUQ","CUS","CV","CVA","CVQ","CW","CWQ","CX","CYQ","CZ","CZ (1)","CZQ","D7","D8","DA","DAN","DAU","DC","DE","DEA","DEC","DFS","DH","DH (1)","DHL","DHQ","DIA","DIC","DIR","DL","DM","DO","DOL","DOQ","DPA","DPI","DQQ","DSA","DST","DUL","DUZ","DVO","DWN","DXA","DY","E0","E7","E72","E8","E9","E9 (1)","EA","EAI","EAQ","EAS","EAT","EBQ","ECA","ECI","ECL","ECQ","ECR","ED","EDE","EE","EET","EEX","EEZ","EFA","EFQ","EGA","EGJ","EGL","EH","EHA","EI","EIA","EJQ","EK","EKA","ELL","EM","EMA","EME","EMP","ENT","EOQ","EP","EQ","ER","ERI","ERO","ERQ","ESQ","ET","EU","EUQ","EV","EVA","EW","EWA","EXA","EXC","EXC (1)","EXP","EXQ","EXR","EY","EZ","F2","F8","F9","FAA","FAC","FAQ","FAR","FB","FBI","FC","FCA","FCQ","FD","FDA","FDM","FDQ","FE","FF","FFQ","FH","FI","FJ","FJ (1)","FL","FL (1)","FLA","FLM","FLR","FNQ","FOR","FOS","FP","FQ","FRA","FRS","FRT","FRW","FS","FSA","FT","FTI","FVA","FW","FWA","FX","FYB","G3","G3 (1)","G4","G7","GA","GAL","GAQ","GAS","GAT","GAV","GBA","GBQ","GCA","GCH","GCQ","GCS","GD","GDW","GE","GF","GF (1)","GFC","GFQ","GG","GG (1)","GG (2)","GH","GHA","GIQ","GJ","GJ (1)","GKQ","GL","GLW","GM","GMA","GOF","GPA","GQ","GR","GRA","GRD","GRO","GS","GSA","GSC","GSL","GSQ","GST","GU","GUL","GUN","GV","GW","GW (1)","GWA","GWE","GX","GY","H2","H5","H6","HA","HAQ","HAR","HB","HB (1)","HBQ","HCQ","HEL","HEP","HER","HET","HEU","HFQ","HFS","HIQ","HJ","HK","HLQ","HMZ","HNS","HOL","HOQ","HOR","HP","HPZ","HQ","HQ (1)","HQQ","HRA","HRG","HRQ","HRZ","HSV","HSZ","HU","HUB","HVQ","HWK","HX","HX (1)","HY","HYA","I4","IA","IB","ICF","ICH","ICQ","IDQ","IEX","IG","IGA","III","IJ","IKQ","IMP","IN","INA","IND","INQ","IOW","IPI","IPQ","IPS","IR","IRQ","ISL","ITQ","IW","IW (1)","IXQ","J2","J5","J6","J7","JAG","JAM","JAQ","JB","JCM","JCQ","JCZ","JD","JD (1)","JER","JF","JI","JI (1)","JIQ","JJ","JK","JK (1)","JKQ","JL","JM","JMA","JN","JO","JQ","JR","JTA","JU","JU (1)","JW","JX","JX (1)","JXQ","JZ","K2","K3","K5","K7","K8","KA","KAH","KAI","KAQ","KAT","KB","KD","KE","KEA","KEE","KET","KFS","KH","KI","KIN","KKQ","KL","KLQ","KM","KN","KO","KOD","KP","KR","KR (1)","KS","KTQ","KU","KV","KW","KWQ","KWZ","KX","KXZ","KZ","L2","L3","L7","LA","LAA","LAH","LAN","LAP","LAR","LAS","LAW","LB","LBQ","LC","LC (1)","LC (2)","LCQ","LCT","LDM","LEQ","LEX","LF","LF (1)","LG","LGQ","LH","LHA","LIQ","LJ","LKN","LLA","LNQ","LO","LOQ","LOS","LP","LP (1)","LR","LRQ","LS","LSQ","LSZ","LT","LU","LUQ","LVA","LW","LX","LXQ","LY","LZ","LZQ","M3","M6","M7","MA","MAA","MAC","MAE","MAG","MAI","MAQ","MAR","MAV","MAX","MB","MC","MCO","MDC","MDU","ME","MEA","MET","MEX","MF","MG","MGQ","MH","MHK","MHO","MIN","MIS","MIY","MJ","MKQ","ML","ML (1)","MLL","MLX","MM","MM (1)","MMH","MNA","MNZ","MOQ","MP","MPA","MPC","MQ","MRC","MRR","MS","MST","MT","MT (1)","MTA","MTR","MTV","MTX","MU","MU (1)","MUA","MUQ","MVA","MW","MWA","MX","MY","MYQ","MZ","N5","N5 (1)","N6","N7","N8","NA","NAC","NAP","NAQ","NAS","NAT","NB","NBQ","NC","NCA","NCE","ND","NEB","NEC","NEQ","NER","NET","NEV","NEW","NEX","NFA","NFQ","NG","NGL","NH","NHA","NI","NI (1)","NJ","NJA","NJE","NK","NKQ","NLQ","NN","NOQ","NPA","NPT","NQ","NRN","NSQ","NTH","NTQ","NUM","NUQ","NVQ","NVX","NW","NWQ","NWS","NXQ","NY","NYH","NZ","O6","OA","OAK","OB","OC","OCR","OD","OD (1)","OE","OF","OFF","OH","OH (1)","OHA","OHZ","OI","OIZ","OJ","OK","OMH","OMI","OMK","OMN","OMQ","ON","ONQ","OO","OOZ","OP","OP (1)","OPQ","OR","ORI","OS","OTT","OV","OW","OWQ","OWS","OZ","OZ (1)","OZA","P3","P9","PA","PA (1)","PA (2)","PAI","PAQ","PAT","PBA","PBQ","PCA","PCL","PCQ","PCR","PD","PD (1)","PDA","PDQ","PE","PE (1)","PEA","PEQ","PER","PET","PFA","PFQ","PH","PHL","PHR","PHX","PI","PIA","PIQ","PK","PKQ","PL","PLA","PLQ","PLZ","PM","PMA","PMT","PN","PNM","PNQ","PNR","PNS","PO","POA","POC","POL","POM","PON","POQ","PP","PPQ","PR","PRA","PRD","PRM","PRN","PRO","PRP","PRQ","PRR","PS","PS (1)","PSA","PT","PT (1)","PTQ","PU","PUQ","PV","PVA","PW","PY","PZ","Q2","Q4","Q5","Q7","Q7 (1)","QB","QD","QF","QG","QH","QK","QN","QO","QOZ","QQ","QR","QS","QS (1)","QT","QTQ","QWE","QX","R2","R3","RAL","RAN","RAQ","RAY","RB","RC","RCA","RD","RD (1)","RDR","RDW","REA","RED","REQ","RES","RG","RGQ","RHA","RIA","RIC","RID","RIO","RIQ","RIV","RJ","RJS","RK","RL","RL (1)","RLQ","RMH","RMV","RNQ","RO","ROA","ROE","ROQ","ROS","ROY","RP","RRT","RSC","RSQ","RSV","RTQ","RU","RV","RV (1)","RVA","RVQ","RWG","RY","RZZ","S3","S4","S5","S6","S6 (1)","SA","SAC","SAH","SAI","SAJ","SAL","SAM","SAQ","SAS","SAX","SBA","SBN","SBQ","SCN","SCO","SCT","SCY","SDQ","SE","SE (1)","SEB","SEG","SEM","SEP","SEQ","SER","SFI","SFO","SFQ","SFQ (1)","SFS","SG","SH","SHS","SHV","SI","SI (1)","SIA","SIQ","SIQ (1)","SJA","SJQ","SK","SKI","SKJ","SKL","SKM","SKS","SKT","SLO","SLQ","SLR","SLZ","SM","SMC","SMO","SMQ","SN","SN (1)","SNA","SNB","SNC","SNE","SNK","SNV","SO","SOA","SOE","SOL","SOP","SPF","SPG","SPH","SPQ","SPR","SQ","SR","SRA","SRL","SS","SSA","SSP","SSQ","SSS","SST","ST","STA","STF","STG","STQ","STR","STW","STZ","SU","SUA","SUI","SUT","SUX","SV","SVL","SVW","SWA","SWT","SX","SX (1)","SXP","SY","SYB","SYM","SZQ","T3","T9","TA","TAC","TAF","TAL","TAQ","TAS","TAT","TB","TB (1)","TBQ","TCA","TCE","TCI","TCL","TCO","TCQ","TDQ","TDQ (1)","TEN","TER","TFQ","TFZ","TG","TGO","TGQ","TH","THA","THO","THQ","TIA","TIE","TIQ","TIS","TJ","TK","TKQ","TLQ","TMA","TN","TNQ","TNS","TP","TPQ","TQQ","TR","TR (1)","TRA","TRI","TRO","TRP","TRQ","TRR","TS","TSF","TSI","TSQ","TTM","TUL","TUR","TV","TV (1)","TVL","TW","TWA","TWE","TX","TX (1)","TXA","TXE","TXQ","TXY","TYE","TZ","TZQ","U2","U5","U7","UA","UAI","UC","UD","UH","UMQ","UN","UNQ","UO","UP","UR","URQ","US","USA","UT","UTQ","UX","UX (1)","UYQ","V4","V5","V8","V9","VA","VA (1)","VAG","VAQ","VAS","VB","VB (1)","VC","VCQ","VE","VER","VEZ","VH","VI","VIG","VIN","VIQ","VIQ (1)","VIS","VJT","VK","VL","VLA","VLY","VOQ","VP","VQ","VR","VS","VSA","VV","VV (1)","VV (2)","VW","VX","VX (1)","VY","W3","W4","W7","W8","W9","WA","WA (1)","WAC","WAI","WAP","WAQ","WAR","WAW","WC","WCA","WCI","WD","WDQ","WE","WEB","WEQ","WES","WET","WFS","WFT","WG","WGL","WGQ","WI","WIA","WIL","WIR","WL","WL (1)","WLH","WLQ","WM","WN","WNG","WNQ","WO","WP","WR","WRA","WRD","WRT","WS","WS (1)","WSK","WST","WSU","WT","WTA","WV","WV (1)","WW","WWI","WWQ","X4","X9","XAA","XAB","XAC","XAE","XAF","XAG","XAH","XAI","XAJ","XAK","XAM","XAO","XAQ","XAR","XAS","XAT","XAU","XAY","XAZ","XBC","XBF","XBH","XBI","XBJ","XBM","XBP","XBQ","XBR","XBT","XBV","XBX","XBZ","XC","XCA","XDA","XDC","XDD","XDE","XDF","XDH","XE","XF","XJ","XK","XL","XP","XPA","XPQ","XV","XY","XYZ","Y2","Y4","Y7","Y8","YB","YBQ","YM","YR","YV","YX","YX (1)","Z3","Z3 (1)","Z3Q","ZA","ZAQ","ZB","ZE","ZE (1)","ZFZ","ZIA","ZK","ZKQ","ZL","ZMZ","ZN","ZO","ZP","ZS","ZUQ","ZV","ZW","ZX","ZX (1)","ZY","ZYZ"]
                }
            }*/
        ]
    },

    // old config
    /*"twitter-small": {
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
    }*/
};
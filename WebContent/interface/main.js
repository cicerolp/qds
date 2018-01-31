console.log("EVV");

//widgets
var myMap           = undefined;
var boxPlotWidget   = undefined;
var equidepthWidget = undefined;

//constraints
var initialTimeConstraint = {"lower":1205971200,"upper":1287014400};
var timeConstraint = {"lower":1205971200,"upper":1287014400};
var geoConstraints = [];

//vars
var xmlFile        = "data/example.xml";
var ndsInterface   = undefined;
var datasetName    = undefined;
var datasetSchema  = {};
var mapIndexToName = {};

var temporalDimension = 3;
var spatialDimension  = 0; 

/*******************
 * Query Functions *
 *******************/

function queryEquiDepthPlot(){
    //http://localhost:7000/api/query/dataset=brightkite/aggr=quantile.(0:0.25:0.5:0.75:1.0)/const=1.values.(all)/group=1
    var q = new NDSQuery(datasetName,"quantile",1,
			 function(result){
			     var numEntries = result.length;
			     //{"lower":0,"upper":0.1  ,"density":0.25}
			     var data = [];
			     if(numEntries > 0){
 				 //
				 var prevEntry = result[0];
				 var bins = [];
				 for(var i = 1 ; i < numEntries ; ++i){
				     var entry = result[i];
				     if(prevEntry[0] == entry[0]){
					 bins.push({"lower":prevEntry[2],"upper":entry[2],"density":(entry[1]-prevEntry[1])});
				     }
				     else{
					 data.push({"label":prevEntry[0],"bins":bins});
					 bins = [];
				     }

				     prevEntry = entry;
				 }
				 data.push({"label":prevEntry[0],"bins":bins});

				 //TODO:normalize
			     }
			     equidepthWidget.setData(data);
			 });
    q.setPayload({"quantiles":d3.range(11).map(d=>0.1*d)});
    //
    q.addConstraint("categorical",1,{"values":["all"]});
    if(geoConstraints.length > 0){
	q.addConstraint("region",spatialDimension,{"zoom":myMap.map.getZoom(),"geometry":[geoConstraints[0].geometry[0], geoConstraints[0].geometry[1],geoConstraints[0].geometry[4],geoConstraints[0].geometry[5]]});
    }
    //
    ndsInterface.query(q);

}

function queryBandPlot(){
    //http://localhost:7000/api/query/dataset=brightkite/aggr=quantile.(0:0.25:0.5:0.75:1.0)/const=1.values.(all)/group=1
    var q = new NDSQuery(datasetName,"quantile",temporalDimension,
			 function(result){
			     console.log(result);
			     var numEntries = result.length;
			     //{"lower":0,"upper":0.1  ,"density":0.25}
			     var numQuantiles = this.payload.quantiles.length;
			     var data = d3.range(numQuantiles).map(d=>[]);
			     console.log(result);
 			     if(numEntries > 0){
				 for(var i = 0 ; i < numEntries ; ++i){
				     data[i%numQuantiles].push([result[i][0],result[i][2]])
				 }
			     }
			     
			     //
			     var numCurves = data.length;
			     var numBands = Math.floor(numCurves/2);

			     var bands = [];
			     
			     for(var i = 0 ; i < numBands ; ++i){
				 var upperLeft  = data[i][0];
				 var bandData = data[i].concat(data[numCurves-1-i].reverse()).concat([upperLeft]);

				 bands.push(bandData);
			     }
			     
			     //
			     bandPlotWidget.setData(bands,data[Math.floor(numCurves/2)]);
			 });
    q.setPayload({"quantiles":[0,0.25,0.5,0.75,1.0]});
    //
    q.addConstraint("time_interval",temporalDimension,timeConstraint);
    if(geoConstraints.length > 0){
	q.addConstraint("region",spatialDimension,{"zoom":myMap.map.getZoom(),"geometry":[geoConstraints[0].geometry[0], geoConstraints[0].geometry[1],geoConstraints[0].geometry[4],geoConstraints[0].geometry[5]]});
    }
    //
    ndsInterface.query(q);

}

function updateSystem(){
    queryBoxPlot();
}

/*************************
 * set control variables *
 *************************/

function changeMapMode(e){
    //
    var newMode = d3.event.target.getAttribute("value");
    var ndsLayer = myMap.getLayer("ndsLayer");
    ndsLayer.setMode(newMode);
    //
    if(newMode == "inverse_quantile"){
	var heatmapColorScale = d3.scaleOrdinal().domain([0, 0.14285714285714285, 0.2857142857142857, 0.42857142857142855, 0.5714285714285714, 0.7142857142857142, 0.8571428571428571, 1]).range(['#ffffcc','#ffeda0','#fed976','#feb24c','#fd8d3c','#fc4e2a','#e31a1c','#b10026']);
	myMap.updateLegend(d3.scaleLinear().range(["white","red"]),"continuousLog","Quantile","#legend");
	d3.select("#legend").style("display", "block");
    }
    else{
	d3.select("#legend").style("display", "none");
    }
}

/**********************
 * Handle Constraints *
 **********************/

function mapSelectionChanged(mapConstraints){
    geoConstraints = mapConstraints.constraints;
    //
    queryEquiDepthPlot();
    queryBandPlot();
}

function setTemporalConstraint(constraint){
    console.log("====>",constraint);
    if(constraint == null){
	timeConstraint = initialTimeConstraint;
    }
    else{
	timeConstraint.lower = constraint.constraints[0];
	timeConstraint.upper = constraint.constraints[1];
    }

    console.log("constraint",constraint);
    
    //
    queryBoxPlot();
    queryEquiDepthPlot();
    var ndsLayer = myMap.getLayer("ndsLayer");
    ndsLayer.repaint();
}

/********
 * Main *
 ********/

function initializeSystem(){
    //add map
    var mapID = "mapID";
    d3.select("body").append("div").attr("id",mapID);    
    var tileURL = 'http://{s}.tiles.wmflabs.org/bw-mapnik/{z}/{x}/{y}.png';
    var tileLayerProperties = {
	maxZoom: 18,
	attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
    };
    myMap = new GLLeafletMap(mapID,[-14.408656850000002,-51.31668], 4, tileURL, tileLayerProperties);
    myMap.setCallback("selectionChanged",mapSelectionChanged);
    var ndsLayer = new NDSLayer(myMap.map,ndsInterface);
    myMap.addLayer(ndsLayer,"ndsLayer");
    d3.select("#legend").style("display", "none");
    d3.select("#legendSeq").style("display", "none");
    
    //
    var levelSlider = d3.select("#geoLevelSlider");
    levelSlider.on("change",function(){
	var geoDiveLevel = +d3.event.target.value;
	ndsLayer.setResolution(geoDiveLevel);
	// queryGeo();
    });
    var heatmapOppacitySlider = d3.select("#heatmapOppacitySlider");
    heatmapOppacitySlider.on("input",function(){
	var newValue = +d3.event.target.value;
	ndsLayer.setOpacity(newValue/255.0);
    });

    //
    var modeOptions = d3.select("#modeDiv")
	.selectAll("input")
	.on("change",changeMapMode);
    
    //
    // var formQueryPlaceDiv = d3.select("body").append("div").attr("class","placeForm").attr("id","placeFeaturesDiv");
    // var optionsDiv = formQueryPlaceDiv.append("div").attr("id","optionsDiv");
    // var divFunctions = [{"id":"Quantile","screenName":"Quantile"}];
    // var divs = formQueryPlaceDiv
    // 	.selectAll("div")
    // 	.data(divFunctions)
    // 	.enter()
    // 	.append("div")
    // 	.attr("id",d=>("funcDiv" + d.id));
    // divs.append("label")
    // 	.attr("for",d=>("funcDiv" + d.id))
    // 	.text(d=>(d.screenName + ":  "));
    // divs.append("input")
    // 	.attr("type","range")
    // 	.attr("min",0)
    // 	.attr("max",100)
    // 	.attr("step",10)
    // 	.attr("value",50)
    // 	.attr("id",d=>("input" + d.id))
    // 	.on("change",function(e){
    // 	    var newValue = +d3.event.target.value;
    // 	    ndsLayer.setQuantileMapQuantile((newValue/100.0));
    // 	});

    //
     d3.select("#quantileSlider").on("change",function(d){
	 var newValue = +d3.event.target.value;
	 d3.select("#quantileLabel").text(newValue/100.0);
	 ndsLayer.setQuantileMapQuantile((newValue/100.0));
    });
    //
    d3.select("#inverseQuantileSlider").on("change",function(d){
	var newValue = +d3.event.target.value;
	d3.select("#inverseQuantileLabel").text(newValue);
	ndsLayer.setInverseQuantileMapQuantile((newValue));
    });
    

    
    //add boxplot histogram
    // var boxPlotWidgetDiv = d3.select("body").append("div").attr("id","boxPlotWidget");
    // boxPlotWidget = new BoxPlotWidget(boxPlotWidgetDiv,"boxPlotWidget");

    //add equidepth histogram
    var equidepthWidgetDiv = d3.select("body").append("div").attr("id","equidepthWidget");
    equidepthWidget = new EquidepthWidget(equidepthWidgetDiv,"equidepthWidgetDiv");

    //add equidepth histogram
    var bandWidgetDiv = d3.select("body").append("div").attr("id","bandWidget");
    bandPlotWidget = new BandPlotWidget(bandWidgetDiv,"bandWidgetDiv",setTemporalConstraint);

    
    //
    // var timeSeriesDiv = d3.select("body").append("div").attr("id","timeSeriesWidget");
    // timeSeriesWidget = new TimeSeriesWidget(equidepthWidgetDiv,"equidepthWidgetDiv");
    
    //
    queryBoxPlot();
    queryEquiDepthPlot();
    queryBandPlot();
}

/*******************
 * Start Execution *
 *******************/

d3.xml(xmlFile, function(error, data) {

    var schemaOBJ = xmlToJson(data);
    datasetName = schemaOBJ.config.output["#text"];
    datasetName = "brightkite";

    //process schema
    schemaOBJ.config.schema.categorical.forEach(dimension=>{
	var attributes = dimension["@attributes"];
	if(attributes.type == "discrete"){
	    attributes["bins"] = dimension.bins.bin.map(function(d,i){ return {"key":d.key,"index":i} });
	}
	else if(attributes.type == "sequential"){
	    attributes["bins"] = dimension.bins.bin;
	}
	datasetSchema[attributes.name] = attributes;
	mapIndexToName[attributes.index] = attributes.name;
    });

    //temporal
    var temporalAttributes = schemaOBJ.config.schema.temporal["@attributes"];
    temporalAttributes["min"] = schemaOBJ.config.schema.temporal.bins.bin.min;
    temporalAttributes["max"] = schemaOBJ.config.schema.temporal.bins.bin.min;
    datasetSchema[temporalAttributes.name] = temporalAttributes;
    mapIndexToName[temporalAttributes.index] = temporalAttributes.name;

    //spatial
    var spatialAttributes = schemaOBJ.config.schema.spatial["@attributes"];   
    datasetSchema[spatialAttributes.name] = spatialAttributes;
    mapIndexToName[spatialAttributes.index] = spatialAttributes.name;

    //
    ndsInterface = new NDSInterface("localhost",7000,d=>{console.log("creation of NDS interface done!")});
    initializeSystem();

});



/******************
 * test functions *
 ******************/


function test(){
    var q = new NDSQuery(datasetName,"count","",(d,q)=>{console.log(d,q);});

    ndsInterface.query(q);

    var myDate0 = new Date(2018,1,1);
    var myDate1 = new Date(2018,10,1);
    q.addConstraint("time_interval","time",{"lower":myDate0.getTime()/1000,"upper":myDate1.getTime()/1000});
    ndsInterface.query(q);
}

// 
function testBoxPlot(){
    var data = [["entry1",0,1,2,4,5],
		["entry2",0.5,1.5,2,4,10],
		["entry3",3,4,9,9.5,15]];

    boxPlotWidget.setData(data);
}

//
function testEquiDepth(){

    var data = [{"label":"entry1","bins":[{"lower":0,"upper":0.1  ,"density":0.25},
					  {"lower":0.1,"upper":0.5,"density":0.25},
					  {"lower":0.5,"upper":0.9,"density":0.25},
					  {"lower":0.9,"upper":1.0,"density":0.25}]},
		
		{"label":"entry2","bins":[{"lower":0.1,"upper":0.2,"density":0.4},
					  {"lower":0.2,"upper":0.3,"density":0.05},
					  {"lower":0.3,"upper":0.4,"density":0.1},
					  {"lower":0.4,"upper":0.5,"density":0.05},
					  {"lower":0.5,"upper":0.6,"density":0.4},
					 ]
		},	    
		{"label":"entry3","bins":[{"lower":0.5,"upper":1.0,"density":0.75},
					  {"lower":1.0,"upper":2.0,"density":0.15},
					  {"lower":2.0,"upper":2.5,"density":0.1}
					 ]
		}
	       ];
    
    equidepthWidget.setData(data);
}

/***********
 * scratch *
 ***********/

function queryBoxPlot(){
    //http://localhost:7000/api/query/dataset=brightkite/aggr=quantile.(0:0.25:0.5:0.75:1.0)/const=3.interval.(1205971200:1287014400)/group=3
    if(boxPlotWidget == undefined)
	return;
    
    var q = new NDSQuery(datasetName,"quantile",temporalDimension,
			 function(result){
			     var prevValue = undefined;
			     var currentEntry = [];
			     var data = [];
			     result.forEach(function(entry){
				 
				 if(entry[0] == prevValue){
				     currentEntry.push(entry[2]);
				 }
				 else{
				     if(prevValue != undefined){
					 data.push(currentEntry);
				     }

				     //
				     currentEntry = [entry[0],entry[2]];
				     prevValue = entry[0];
				 }
			     });
			     data.push(currentEntry);
			     boxPlotWidget.setData(data);
			 });
    q.setPayload({"quantiles":[0,0.25,0.5,0.75,1.0]});
    console.log("======>",timeConstraint);
    q.addConstraint("time_interval",temporalDimension,timeConstraint);
    console.log(q.toString());
    ndsInterface.query(q);

}

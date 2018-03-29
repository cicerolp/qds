//widgets
var myMap           = undefined;
var boxPlotWidget   = undefined;
var equidepthWidget = undefined;

//constraints
var initialTimeConstraint = undefined;
var timeConstraint = undefined;
var datasetTimeStep       = 0; 
var geoConstraints = [];

//vars
var currentDataset = "flights"
var datasetInfo    = undefined;
var ndsInterface   = undefined;

//
var activeCategoricalDimension = undefined;
var activeTemporalDimension    = undefined; 
var activeSpatialDimension     = undefined;
var activePayloadDimension     = undefined;

//var mapIndexToName = {"177":"AA","302":"Alaska Airlines","343":"JetBlue","526":"Delta", "714":"Hawaiian Airlines","1067":"SkyWest","1432":"United","1444":"US Airways","1489":"Virgin America","1529":"Southwest"};

var mapIndexToName = {"526":"Delta","302":"Alaska Airlines","1432":"United","1067":"SkyWest","177":"AA","1529":"Southwest","343":"JetBlue"};


//
//http://localhost:7000/api/query/dataset=on_time_performance/aggr=inverse.arr_delay_t.(15)/const=crs_dep_time.interval.(1483239600:1509418800)/const=unique_carrier.values.(177:302:343:526:1067:1432:1529)//group=unique_carrier
var inverseQuantileScale = d3.scaleQuantile().domain([0,1]).range(d3.schemeRdBu[7]);

function getAlias(dimension,value){
    return datasetInfo.aliases[dimension][value];
}

function queryBoxPlot(){
    var q = new NDSQuery(datasetInfo.datasetName,activeCategoricalDimension,
			 function(queryReturn){
			     //debugger
			     var counts = {};
			     if(queryReturn.length != 2){
				 console.log("error")
				 return;
			     }
			     queryReturn[1].forEach(d=>{counts[d[0]]=d[1]});
			     //console.log("totalcounts",counts);
			     var result = queryReturn[0];
			     var numEntries = result.length;
			     //{"lower":0,"upper":0.1  ,"density":0.25}
			     var data = [];

			     if(numEntries > 0){
				 if(true){
				     console.log(queryReturn);
				     //
				     var currentKey = result[0][0];
				     var currentBin = [mapIndexToName[result[0][0]],result[0][2]];
				     var bins = [];
				     
				     for(var i = 1 ; i < numEntries ; ++i){
					 var entry = result[i];
					 if(currentKey == entry[0]){
					     currentBin.push(entry[2]);
					 }
					 else{
					     bins.push(currentBin);
					     label = (entry[0] in mapIndexToName)?mapIndexToName[entry[0]]:entry[0]
					     currentBin = [label,entry[2]];
					     currentKey = entry[0];
					 }
				     }
				     
				     bins.push(currentBin);				    
				     data = bins;
				 }
				 else{
				     //
				     var prevEntry = result[0];
				     var bins = [];
				     for(var i = 1 ; i < numEntries ; ++i){
					 var entry = result[i];
					 var totalCount = counts[entry[0]];
					 
					 bins.push({"lower":prevEntry[1],"upper":entry[1],"density":(1.0/(1+entry[1]-prevEntry[1]))});
					 prevEntry = entry;
				     }
				     console.log("bins",bins.map(d=>d.density));
				     data.push({"label":"0","bins":bins});
				 }
				 //TODO:normalize
			     }


			     
			     if(false){
				 data = data.map(d=>{
				     d.label = getAlias(activeCategoricalDimension,d.label);
				     return d;
				 })
			     }



			     
			     //boxPlotWidget.setYAxisLabel(datasetInfo.payloadsScreenNames[activePayloadDimension]);
			     console.log("=======+>",data);
			     data = [data[2],data[6],data[0],data[4],data[5],data[1],data[3]];
			     boxPlotWidget.setData(data);
			 });
    q.addAggregation("quantile",activePayloadDimension + "_t");
    q.addAggregation("count");
    //q.setPayload({"quantiles":d3.range(11).map(d=>0.1*d)});
    q.setPayload({"quantiles":d3.range(5).map(d=>0.25*d)});
    //q.setPayload({"quantiles":d3.range(5).map(d=>0.25*d)});
    //
    q.addConstraint("time_interval",activeTemporalDimension,timeConstraint);
    q.addConstraint("categorical",activeCategoricalDimension,{"values":Object.keys(mapIndexToName)});
    //q.addConstraint("categorical",activeCategoricalDimension,{"values":["all"]});
    if(geoConstraints.length > 0){
	q.addConstraint("region",activeSpatialDimension,{"zoom":19,"geometry":[geoConstraints[0].geometry[4], geoConstraints[0].geometry[1],geoConstraints[0].geometry[0],geoConstraints[0].geometry[5]]});
    }
    //
    // console.log("===>",activePayloadDimension);
    //console.log(q.toString());
    ndsInterface.query(q);


}

function updateBoxPlot(data){
    debugger
}

/*******************
 * Query Functions *
 *******************/

function queryEquiDepthPlot(){
    //http://localhost:7000/api/query/dataset=green_tripdata_2013/aggr=count/aggr=quantile.(0.5)/const=payment_type.values.(all)/group=payment_type
    //[[[0,354622],[1,728901],[2,4475],[3,3180]],[[0,5.0,29.94832992553711],[1,5.0,32.01000213623047],[2,5.0,35.29999923706055],[3,5.0,31.049999237060548]]]
    var q = new NDSQuery(datasetInfo.datasetName,activeCategoricalDimension,
			 function(queryReturn){
			     //debugger
			     var counts = {};
			     if(queryReturn.length != 2){
				 console.log("error")
				 return;
			     }
			     queryReturn[1].forEach(d=>{counts[d[0]]=d[1]});
			     //
			     var _values = Object.values(counts);
			     var _sum = 0;
			     _values.forEach(d=>{_sum += d;})
			     console.log("**** totalcounts",_sum);
			     //
			     var result = queryReturn[0];
			     var numEntries = result.length;
			     //{"lower":0,"upper":0.1  ,"density":0.25}
			     var data = [];

			     if(numEntries > 0){
				 if(true){
				     console.log(queryReturn);
				     //
				     var prevEntry = result[0];
				     var bins = [];
				     
				     for(var i = 1 ; i < numEntries ; ++i){
					 var entry = result[i];
					 if(prevEntry[0] == entry[0]){
					     var totalCount = counts[entry[0]];
					     bins.push({"lower":prevEntry[2],"upper":entry[2],"density":(1.0/(1+entry[2]-prevEntry[2]))});
					 }
					 else{
					     data.push({"label":mapIndexToName[prevEntry[0]],"bins":bins});
					     bins = [];
					 }

					 prevEntry = entry;
				     }
				     
				     data.push({"label":mapIndexToName[prevEntry[0]],"bins":bins});
				 }
				 else{
				     //
				     var prevEntry = result[0];
				     var bins = [];
				     for(var i = 1 ; i < numEntries ; ++i){
					 var entry = result[i];
					 var totalCount = counts[entry[0]];
					 
					 bins.push({"lower":prevEntry[1],"upper":entry[1],"density":(1.0/(1+entry[1]-prevEntry[1]))});
					 prevEntry = entry;
				     }
				     console.log("bins",bins.map(d=>d.density));
				     data.push({"label":"0","bins":bins});
				 }
				 //TODO:normalize
			     }


			     
			     if(false){
				 data = data.map(d=>{
				     d.label = getAlias(activeCategoricalDimension,d.label);
				     return d;
				 })
			     }



			     
			     equidepthWidget.setYAxisLabel(datasetInfo.payloadsScreenNames[activePayloadDimension]);
			     console.log(data);
			     equidepthWidget.setData(data);
			 });
    q.addAggregation("quantile",activePayloadDimension + "_t");
    q.addAggregation("count");
    q.setPayload({"quantiles":d3.range(11).map(d=>0.1*d)});
    //q.setPayload({"quantiles":d3.range(5).map(d=>0.25*d)});
    //q.setPayload({"quantiles":d3.range(5).map(d=>0.25*d)});
    //
    q.addConstraint("time_interval",activeTemporalDimension,timeConstraint);
    q.addConstraint("categorical",activeCategoricalDimension,{"values":Object.keys(mapIndexToName)});
    //q.addConstraint("categorical",activeCategoricalDimension,{"values":["all"]});
    if(geoConstraints.length > 0){
	q.addConstraint("region",activeSpatialDimension,{"zoom":19,"geometry":[geoConstraints[0].geometry[4], geoConstraints[0].geometry[1],geoConstraints[0].geometry[0],geoConstraints[0].geometry[5]]});
    }
    //
    // console.log("===>",activePayloadDimension);
    //console.log(q.toString());
    ndsInterface.query(q);

}

function completeCurve(curve,defaultMinTime,defaultMaxTime,step,auxFactor){
    
    if(auxFactor == undefined)
	auxFactor = 1;

    // if(true){
    // 	var newCurve = [];
    // 	d3.range(curve.length).forEach(i=>{
    // 	    newCurve.push([auxFactor*curve[i][0],curve[i][1]]);
    // 	});
    // 	return newCurve;
    // }
    
    if(curve.length == 0){
	var minTime = defaultMinTime;
	var maxTime = defaultMaxTime;
	var newCurve = [];
	for(var i = minTime ; i <= maxTime ; i += step){
	    newCurve.push([auxFactor*i,0]);
	}
	return newCurve;
    }
    else if(curve.length == 1){
	var minTime = defaultMinTime;
	var maxTime = defaultMaxTime;
	var newCurve = [];
	var onlyTime = curve[0][0];
	for(var t = minTime ; t <= maxTime ; t += step){
	    var value = 0;
	    if(t == onlyTime)
		value = curve[0][1];
	    
	    newCurve.push([auxFactor*t,value]);
	}
	return newCurve;
    }
    else{
	//
	var minTime = curve[0][0];
	var maxTime = curve[curve.length-1][0];
	var newCurve = [];
	var auxIndex = 0;
	//
	for(var t = minTime ; t <= maxTime ; t += step){
	    var value = 0;
	    if(t == curve[auxIndex][0]){
		value = curve[auxIndex][1];
		auxIndex += 1;
	    }	    
	    newCurve.push([auxFactor*t,value]);
	}
	return newCurve;
    }
}


function queryBandPlot(){
    //http://localhost:7000/api/query/dataset=green_tripdata_2013/aggr=count/const=pickup_datetime.interval.(1375315200:1388534400)/group=pickup_datetime
    var q = new NDSQuery(datasetInfo.datasetName,activeTemporalDimension,
			 function(queryReturn){
			     var result = queryReturn[0];
			     var numEntries = result.length;
			     //{"lower":0,"upper":0.1  ,"density":0.25}
			     var numQuantiles = this.payload.quantiles.length;
			     var data = d3.range(numQuantiles).map(d=>[]);
 			     if(numEntries > 0){
				 for(var i = 0 ; i < numEntries ; ++i){
				     data[i%numQuantiles].push([result[i][0],result[i][2]])
				 }
			     }
			     
			     //
			     data = data.map(curve=>completeCurve(curve,initialTimeConstraint.lower,initialTimeConstraint.upper,datasetInfo.timeStep,1000));
			     
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
			     var medianCurve = undefined;

			     //
			     if(result.length == 0){
				 var step = datasetInfo.timeStep;				 
				 var minTime = timeConstraint.lower;
				 var maxTime = timeConstraint.upper;
				 medianCurve = [];
				 for(var i = minTime ; i <= maxTime ; i+=step){
				     medianCurve.push([i,0]);
				 }
				 bands = [];
			     }
			     else{
				 medianCurve = data[Math.floor(numCurves/2)];
			     }

			     //
			     var auxCurve = queryReturn[1];
			     var averageCurve = completeCurve(auxCurve,initialTimeConstraint.lower,initialTimeConstraint.upper,datasetInfo.timeStep,1000)

			     //
			     bandPlotWidget.setYAxisLabel(datasetInfo.payloadsScreenNames[activePayloadDimension]);
			     bandPlotWidget.setData(bands,[{"curve":medianCurve,"color":"black"},
			      				   {"curve":averageCurve,"color":"blue"}]);
			     //bandPlotWidget.setData(bands,[{"curve":medianCurve,"color":"black"}]);
			 });
    //
    q.addAggregation("quantile",activePayloadDimension + "_t");
    q.setPayload({"quantiles":[0.1,0.25,0.5,0.75,0.9]});
    q.addAggregation("average",activePayloadDimension + "_g");
    //
    q.addConstraint("time_interval",activeTemporalDimension,timeConstraint);
    if(geoConstraints.length > 0){
	q.addConstraint("region",activeSpatialDimension,{"zoom":19,"geometry":[geoConstraints[0].geometry[4], geoConstraints[0].geometry[1],geoConstraints[0].geometry[0],geoConstraints[0].geometry[5]]});
    }
    //
    //console.log(q.toString());
    ndsInterface.query(q);
}

function updateSystem(){
    queryBandPlot();
    queryEquiDepthPlot();
    queryBoxPlot();
    var ndsLayer = myMap.getLayer("ndsLayer");
    ndsLayer.repaint();
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
	myMap.updateLegend(inverseQuantileScale,"continuousLog","Quantile","#legend");
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
    queryBoxPlot();
}

function setTemporalConstraint(constraint){
    if(constraint == null){
	timeConstraint = initialTimeConstraint;
    }
    else{
	timeConstraint.lower = constraint.constraints[0];
	timeConstraint.upper = constraint.constraints[1];
    }

    console.log("constraint",constraint);
    //
    queryEquiDepthPlot();
    var ndsLayer = myMap.getLayer("ndsLayer");
    ndsLayer.repaint();
}

function updateTimeConstraint(e){
    var lower = $('#datetimepicker1').data("DateTimePicker").viewDate().unix();
    var upper = $('#datetimepicker2').data("DateTimePicker").viewDate().unix();
    setTemporalConstraint({"constraints":[lower,upper]});
    queryBandPlot();
}

/********
 * Main *
 ********/

function initializeSystem(){
    //
    $('#datetimepicker1').datetimepicker({"inline": true}).data("DateTimePicker").date(new Date(1375315200000)).hide();
    $('#datetimepicker1').on("dp.change",updateTimeConstraint);
     $('#datetimepicker2').datetimepicker({"inline": true}).data("DateTimePicker").date(new Date(1388534400000)).hide();
     $('#datetimepicker2').on("dp.change",updateTimeConstraint);
    
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
    d3.select("#payloadCombobox")
	.on("change",function(d){
	    activePayloadDimension = d3.event.target.selectedOptions[0].text;
	    updateSystem();
	})
	.selectAll("option")
	.data(datasetInfo.payloads)
	.enter()
	.append("option")
	.text(d=>d);
    
    //
    var modeOptions = d3.select("#modeDiv")
	.selectAll("input")
	.on("change",changeMapMode);

    //
     d3.select("#quantileSlider").on("change",function(d){
	 var newValue = +d3.event.target.value;
	 ndsLayer.setQuantileMapQuantile((newValue/100.0));
     }) .on("input",function(d){
     	 var newValue = +d3.event.target.value;
     	 d3.select("#quantileLabel").text(newValue/100.0);
     });
    
    //
    d3.select("#inverseQuantileSlider").on("change",function(d){
	var newValue = +d3.event.target.value;
	d3.select("#inverseQuantileLabel").text(newValue);
	ndsLayer.setInverseQuantileMapQuantile((newValue));
    });

    //
    d3.select("#thresholdSlider").on("change",function(d){
	var newValue = +d3.event.target.value;
	d3.select("#thresholdLabel").text(newValue);
	ndsLayer.setQueryThreshold((newValue));
    });
    

    
    //add boxplot histogram
    var boxPlotWidgetDiv = d3.select("body").append("div").attr("id","boxPlotWidget");
    boxPlotWidget = new BoxPlotWidget(boxPlotWidgetDiv,"boxPlotWidget");

    //add equidepth histogram
    var equidepthWidgetDiv = d3.select("body").append("div").attr("id","equidepthWidget");
    equidepthWidget = new EquidepthWidget(equidepthWidgetDiv,"equidepthWidgetDiv",datasetInfo);
    equidepthWidget.setDimensionSelectionCallback(function(newDimension){
	activeCategoricalDimension = newDimension;
	queryEquiDepthPlot();
    });
    
    //add equidepth histogram
    var bandWidgetDiv = d3.select("body").append("div").attr("id","bandWidget");
    bandPlotWidget = new BandPlotWidget(bandWidgetDiv,"bandWidgetDiv",setTemporalConstraint,datasetInfo);

    
    //
    // var timeSeriesDiv = d3.select("body").append("div").attr("id","timeSeriesWidget");
    // timeSeriesWidget = new TimeSeriesWidget(equidepthWidgetDiv,"equidepthWidgetDiv");
    
    //
    queryBoxPlot();
    queryEquiDepthPlot();
    queryBandPlot();
}

function loadAuxData(){
//    d3.csv(""
}

/*******************
 * Start Execution *
 *******************/
(function(){
    //
    loadAuxData();
    //
    console.log("EVV");
    datasetInfo = datasets[currentDataset];
    activeCategoricalDimension = datasetInfo.categoricalDimension[0];
    activeTemporalDimension    = datasetInfo.temporalDimension[0];
    activeSpatialDimension     = datasetInfo.spatialDimension[0];
    activePayloadDimension     = datasetInfo.payloads[0];
    initialTimeConstraint      = datasetInfo.initialTimeConstraint;
    timeConstraint             = {"lower":initialTimeConstraint.lower,"upper":initialTimeConstraint.upper};
    datasetTimeStep            = datasetInfo.timeStep; 
    
    //
    ndsInterface = new NDSInterface("localhost",7000,d=>{console.log("creation of NDS interface done!")});
    initializeSystem();
})()

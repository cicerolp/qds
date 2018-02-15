//widgets
var myMap           = undefined;
var boxPlotWidget   = undefined;
var equidepthWidget = undefined;

//constraints
var initialTimeConstraint ={ "lower":1375315200,"upper":1388534400};
var timeConstraint = {"lower":1375315200,"upper":1388534400};
var geoConstraints = [];

//vars
var currentDataset = "green_cabs"
var datasetInfo    = undefined;
var ndsInterface   = undefined;

//
var activeCategoricalDimension = undefined;
var activeTemporalDimension    = undefined; 
var activeSpatialDimension     = undefined;
var activePayloadDimension     = undefined;


function getAlias(dimension,value){
    return datasetInfo.aliases[dimension][value];
}

/*******************
 * Query Functions *
 *******************/

function queryEquiDepthPlot(){
    //http://localhost:7000/api/query/dataset=green_tripdata_2013/aggr=count/aggr=quantile.(0.5)/const=payment_type.values.(all)/group=payment_type
    //[[[0,354622],[1,728901],[2,4475],[3,3180]],[[0,5.0,29.94832992553711],[1,5.0,32.01000213623047],[2,5.0,35.29999923706055],[3,5.0,31.049999237060548]]]
    var q = new NDSQuery(datasetInfo.datasetName,activeCategoricalDimension,
			 function(queryReturn){
			     var counts = {};
			     console.log(queryReturn);
			     if(queryReturn.length != 2){
				 console.log("error")
				 return;
			     }
			     queryReturn[1].forEach(d=>{counts[d[0]]=d[1]});
			     console.log("totalcounts",counts);
			     var result = queryReturn[0];
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
					 var totalCount = counts[entry[0]];
					 bins.push({"lower":prevEntry[2],"upper":entry[2],"density":(entry[1]-prevEntry[1])*totalCount});
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
			     data = data.map(d=>{
				 d.label = getAlias(activeCategoricalDimension,d.label);
				 return d;
			     })
			     equidepthWidget.setData(data);
			 });
    q.addAggregation("quantile",activePayloadDimension);
    q.addAggregation("count");
    q.setPayload({"quantiles":d3.range(11).map(d=>0.1*d)});
    //
    q.addConstraint("time_interval",activeTemporalDimension,timeConstraint);
    q.addConstraint("categorical",activeCategoricalDimension,{"values":["all"]});
    if(geoConstraints.length > 0){
	q.addConstraint("region",activeSpatialDimension,{"zoom":19,"geometry":[geoConstraints[0].geometry[4], geoConstraints[0].geometry[1],geoConstraints[0].geometry[0],geoConstraints[0].geometry[5]]});
    }
    //
    console.log(q.toString());
    ndsInterface.query(q);

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
				     data[i%numQuantiles].push([1000*result[i][0],result[i][2]])
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
    //
    q.addAggregation("quantile",activePayloadDimension);
    q.setPayload({"quantiles":[0.25,0.5,0.75]});
    //
    q.addConstraint("time_interval",activeTemporalDimension,timeConstraint);
    if(geoConstraints.length > 0){
	q.addConstraint("region",activeSpatialDimension,{"zoom":19,"geometry":[geoConstraints[0].geometry[4], geoConstraints[0].geometry[1],geoConstraints[0].geometry[0],geoConstraints[0].geometry[5]]});
    }
    //
    ndsInterface.query(q);
}

function updateSystem(){
    //queryBoxPlot();
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
    //queryBoxPlot();
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
    $('#datetimepicker1').datetimepicker({"inline": true}).data("DateTimePicker").date(new Date(1375315200000));
    $('#datetimepicker1').on("dp.change",updateTimeConstraint);
    $('#datetimepicker2').datetimepicker({"inline": true}).data("DateTimePicker").date(new Date(1388534400000));
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
	.selectAll("option")
	.on("change",function(d){
	    activePayloadDimension = d3.event.target.selectedOptions[0].text;
	    updateSystem();
	})
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
    

    
    //add boxplot histogram
    // var boxPlotWidgetDiv = d3.select("body").append("div").attr("id","boxPlotWidget");
    // boxPlotWidget = new BoxPlotWidget(boxPlotWidgetDiv,"boxPlotWidget");

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
    // queryBoxPlot();
    queryEquiDepthPlot();
    queryBandPlot();
}

/*******************
 * Start Execution *
 *******************/
(function(){
    //
    console.log("EVV");
    datasetInfo = datasets[currentDataset];
    activeCategoricalDimension = datasetInfo.categoricalDimension[0];
    activeTemporalDimension    = datasetInfo.temporalDimension[0];
    activeSpatialDimension     = datasetInfo.spatialDimension[0];
    activePayloadDimension     = datasetInfo.payloads[0];
    
    //
    ndsInterface = new NDSInterface("localhost",7000,d=>{console.log("creation of NDS interface done!")});
    initializeSystem();
})()

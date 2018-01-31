
function testEquiDepth(){
    var placeIDDiv = d3.select("body")
	.append("div")
	.attr("id","placeIDWidget");

    //
    var placeIDChartWidth = 800;
    var placeIDChartHeight = 500;

    var svgPlaceIDChart = placeIDDiv
	.append("svg")
	.attr("id","placeIDWidgetSVG")
	.attr("width",placeIDChartWidth)
	.attr("height",placeIDChartHeight);
    var placeIDWidget = new EquiDepthHistogramWidget(svgPlaceIDChart,"placeID",20,20,500,400);//placeIDChartWidth,placeIDChartHeight);

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

    placeIDWidget.setData(data);

    d3.select("body").append("button").text("click").on("click",function(){

	var data = [
	    {"label":"entry2","bins":[{"lower":0.1,"upper":0.2,"density":0.4},
				      {"lower":0.2,"upper":0.3,"density":0.05},
				      {"lower":0.3,"upper":0.4,"density":0.1},
				      {"lower":0.4,"upper":0.5,"density":0.05},
				      {"lower":0.5,"upper":0.6,"density":0.4},
				     ]
	    },
	    {"label":"entry1","bins":[{"lower":0,"upper":0.1  ,"density":0.25},
				      {"lower":0.1,"upper":0.5,"density":0.25},
				      {"lower":0.5,"upper":0.9,"density":0.25},
				      {"lower":0.9,"upper":1.0,"density":0.25}]}
	];


	placeIDWidget.setData(data);    
    })
}

function testBoxPlot(){
    //widgets
    var datasetName = 'brightkite';
    var widgets = {};

    //
    var hourConstraints = [];
    var dayOfWeekConstraints = [];

    //
    function updateHour(){
	
	var query = "http://localhost:7000/rest/query/"+ datasetName + "/count/group/field/2/" + "where/1=" + dayOfWeekConstraints.join(":");
	
	
	d3.request(query).get(updateHourChart);
    }

    function updateDay(){
	var query = "http://localhost:7000/rest/query/" + datasetName + "/count/group/field/1/where/2=" + hourConstraints.join(":");
	
	d3.request(query).get(updateWeekChart);
    }

    function updateBox(){

	//
	var query = "http://localhost:7000/rest/query/"+ datasetName +"/quantile/group/field/2/quantile/0/quantile/0.25/quantile/0.5/quantile/0.75/quantile/1.0/where/2=" + hourConstraints.join(":") + "/" + "where/1=" + dayOfWeekConstraints.join(":");;
	
	d3.request(query).get(updateTimeBoxPlot);
    }

    //
    function hourConstraintChanged(selected){
	hourConstraints = selected;
	updateViews();
	//
	updateDay();
	updateBox();
    }

    function dayConstraintChanged(selected){
	var mapIndexToDayString = {"Mon":0,"Tue":1,"Wed":2,"Thu":3,"Fri":4,"Sat":5,"Sun":6};
	dayOfWeekConstraints = selected;
	dayOfWeekConstraints = dayOfWeekConstraints.map(d=>mapIndexToDayString[d]);

	//
	updateHour();
	updateBox();
    }

    //
    function installBoxPlot(name,container,width,height){
	var svg = container
	    .append("svg")
	    .attr("width",width)
	    .attr("height",height);
	var boxPlotWidget = new BoxPlotWidget(svg,name,0,0,width,height);    
	widgets[name] = boxPlotWidget;
    }

    function installRowChart(name,container,width,height,selectionCallback){
	var svg = container
	    .append("svg")
	    .attr("width",width)
	    .attr("height",height);
	var rowChartWidget = new RowChartWidget(svg,name,0,0,width,height);
	rowChartWidget.setSelectionChangedCallback(selectionCallback);
	widgets[name] = rowChartWidget;
    }

    function updateViews(){
	console.log("update views");
    }

    //
    var boxPlotDiv = d3.select("body").append("div").attr("id","timeBoxplot");
    installBoxPlot("timeBoxPlot",boxPlotDiv,600,300);

    var dayOfWeekDiv = d3.select("body").append("div").attr("id","dayOfWeekPlot");
    installRowChart("dayOfWeekPlot",dayOfWeekDiv,600,300,dayConstraintChanged);

    var hourOfDayDiv = d3.select("body").append("div").attr("id","hourOfDayPlot");
    installRowChart("hourOfDayPlot",hourOfDayDiv,600,500,hourConstraintChanged);

    //
    d3.request("http://localhost:7000/rest/query/brightkite/quantile/group/field/2/region/0/11/402/771/588/871/tseries/3/1205971200/1287014400/quantile/0/quantile/0.25/quantile/0.5/quantile/0.75/quantile/1.0/").get(updateTimeBoxPlot);

    d3.request("http://localhost:7000/rest/query/brightkite/count/group/field/1/").get(updateWeekChart);

    d3.request("http://localhost:7000/rest/query/brightkite/count/group/field/2/").get(updateHourChart);


    function updateTimeBoxPlot(queryResult){
	var response = JSON.parse(queryResult.response);
	console.log(response);

	//
	var key = undefined;
	var keyArray = [];
	var result = [];
	response.forEach(function(d){
	    if(key == undefined){
		key = d[0];
		keyArray.push(key);
		keyArray.push(d[2]);
	    }
	    else if(key === d[0]){
		keyArray.push(d[2]);
	    }
	    else{
		result.push(keyArray);
		key = d[0];
		keyArray = [key];
		keyArray.push(d[2]);
	    }
	});
	result.push(keyArray);
	console.log(result);
	var boxPlotWidget = widgets["timeBoxPlot"];
	boxPlotWidget.setData(result);
    }

    function updateWeekChart(queryResult){
	var response = JSON.parse(queryResult.response);
	
	//
	var mapIndexToDayString = {0:"Mon",1:"Tue",2:"Wed",3:"Thu",4:"Fri",5:"Sat",6:"Sun"};
	var result = response.map(function(d){
    	    return {"key":mapIndexToDayString[d[0]],"value":d[1]};
	});
	var rowChartWidget = widgets["dayOfWeekPlot"];
	rowChartWidget.setData(result);
    }

    function updateHourChart(queryResult){
	var response = JSON.parse(queryResult.response);
	
	//
	var result = response.map(function(d){
    	    return {"key":d[0],"value":d[1]};
	});
	console.log("result",result);
	var rowChartWidget = widgets["hourOfDayPlot"];
	rowChartWidget.setData(result);
    }

}


testBoxPlot();

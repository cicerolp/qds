class BandPlotWidget {
    
    constructor(container,widgetID, timeSelectionCallback){
	//
	this.widgetID  = widgetID;
	this.container = container;
	this.timeSelectionCallback = timeSelectionCallback;
        var controlsContainer = this.container.append("div")
            .attr("class","widgetControls");

	
	var timeSelectionLabel = controlsContainer.append("label").attr("id","timeSeriesSelectionLabel").attr("name","timeSeriesSelectionLabel").text("Select a time range");

	
	//
        var chartWidth = 800;
        var chartHeight = 300;
        var svgCanvas = this.container
            .append("svg")
            .attr("id",widgetID+"_svg")
            .attr("width",chartWidth)
            .attr("height",chartHeight);
        this.plot = new TimeSeriesBandPlot(svgCanvas,widgetID + "_plot",0,0,chartWidth,chartHeight,0);
	this.plot.setTimeSelectionCallBack(this.timeSelectionChanged,this.timeLabelChanged);
    }

    setYAxisLabel(yLabel){
	this.plot.setYLabel(yLabel);
    }
    
    timeSelectionChanged(payload){
	var that = bandPlotWidget;
	if(that.timeSelectionCallback)
	    that.timeSelectionCallback(payload);
    }

    timeLabelChanged(brushSelection){
	var labelString = "Select a time range";
	if(brushSelection){
	    var format = d3.timeFormat("%a %Y-%m-%d %H:%M:%S");
	    var labelString = format(new Date(brushSelection.constraints[0]*1000)) + "  ---  " + format(new Date(brushSelection.constraints[1]*1000));
	}
	//
	d3.select("#timeSeriesSelectionLabel").text(labelString);
    }
    
    setData(bands,curves){
	//curves = [{"curve":[],"color":"red"}]
	this.plot.setData(bands,curves);
    }
}

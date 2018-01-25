class BandPlotWidget {
    
    constructor(container,widgetID){
	//
	this.widgetID  = widgetID;
	this.container = container;
        var controlsContainer = this.container.append("div")
            .attr("class","widgetControls");
        
	//
        var chartWidth = 800;
        var chartHeight = 300;
        var svgCanvas = this.container
            .append("svg")
            .attr("id",widgetID+"_svg")
            .attr("width",chartWidth)
            .attr("height",chartHeight);
        this.plot = new TimeSeriesBandPlot(svgCanvas,widgetID,0,0,chartWidth,chartHeight,0);

    }

    setData(bands,curve){
	this.plot.setData(bands,curve);
    }
}

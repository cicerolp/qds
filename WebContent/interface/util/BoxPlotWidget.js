class BoxPlotWidget {
    constructor(container,widgetID){
	//
	this.widgetID  = widgetID;
	this.container = container;
        var controlsContainer = this.container.append("div")
            .attr("class","widgetControls");
        
	//
        var chartWidth = 800;
        var chartHeight = 250;
        var svgCanvas = this.container
            .append("svg")
            .attr("id",widgetID+"_svg")
            .attr("width",chartWidth)
            .attr("height",chartHeight);
        this.plot = new BoxPlot(svgCanvas,widgetID,0,0,chartWidth,chartHeight);

    }

    setData(data){
	this.plot.setData(data);
    }
}

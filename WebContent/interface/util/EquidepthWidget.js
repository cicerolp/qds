class EquidepthWidget {
    
    constructor(container,widgetID,datasetInfo){
	//
	var widget = this;
	this.dimensionSelectionCallback = undefined;
	//
	this.widgetID  = widgetID;
	this.container = container;
        var controlsContainer = this.container.append("div")
            .attr("class","widgetControls");

	//
	controlsContainer.append("label")
	    .text("Dimension");
	//
	controlsContainer.append("text")
	    .text("                    ")

	//
	controlsContainer.append("select")
	.on("change",function(e){
	    var selectedDimension = d3.event.target.selectedOptions[0].text;
	    if(widget.dimensionSelectionCallback)
		widget.dimensionSelectionCallback(selectedDimension);
	}).selectAll("option")
	    .data(datasetInfo.categoricalDimension)
	    .enter()
	    .append("option")
	    .text(d=>d);
	
	//
        var chartWidth = 800;
        var chartHeight = 250;
        var svgCanvas = this.container
            .append("svg")
            .attr("id",widgetID+"_svg")
            .attr("width",chartWidth)
            .attr("height",chartHeight);
        this.plot = new EquiDepthHistogram(svgCanvas,widgetID,0,0,chartWidth,chartHeight);

    }

    setYAxisLabel(yLabel){
	this.plot.setYAxisLabel(yLabel);
    }
    
    setDimensionSelectionCallback(f){
	this.dimensionSelectionCallback = f;
    }
    
    setData(data){
	this.plot.setData(data);
    }
}

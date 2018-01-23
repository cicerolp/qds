class StackedBarChartWidget{

    constructor(container,widgetID,screenX,screenY,totalWidth,totalHeight){
	//set margins
	this.renderingArea = {x:screenX,y:screenY,width:totalWidth,height:totalHeight};
	this.margins = {left:50,right:10,top:10,bottom:30}
	this.canvasWidth = this.renderingArea.width - this.margins.left - this.margins.right;
	this.canvasHeight = this.renderingArea.height - this.margins.top - this.margins.bottom;
	this.widgetID = widgetID;
	//
	this.data = [];	
	this.stack = undefined;
	this.colorScale = d3.scaleOrdinal();
	//
	this.canvas = container
	    .append("g")
	    .attr("id","line_" + widgetID)
	    .attr("transform","translate("+(this.renderingArea.x+this.margins.left) + ", " + (this.renderingArea.y+this.margins.top) + ")");
	
	//
	this.xScale = d3.scaleBand().range([0,this.canvasWidth]);
	this.xAxis  = d3.axisBottom(this.xScale);
	this.canvas
	    .append("g")
	    .attr("class","xAxis")
	    .attr("transform","translate(0," + this.canvasHeight  + ")");

	//
	this.yScale = d3.scaleLinear().range([this.canvasHeight,0]);
	this.yAxis  = d3.axisLeft(this.yScale);
	this.canvas
	    .append("g")
	    .attr("class","yAxis");

	//
	this.canvas.append("text").attr("id",widgetID + "_labelXAxis");
	this.canvas.append("text").attr("id",widgetID + "_labelYAxis");
	this.xLabel = "";
	this.yLabel = "";
	//
	this.updatePlot();
    }

    setXAxisLabel(xLabel){
	this.xLabel = xLabel;
    }
    
    setYAxisLabel(yLabel){
	this.yLabel = yLabel;
    }
    
    updateAxis(){
	var canvasWidth = this.canvasWidth;
	var canvasHeight = this.canvasHeight;
	
	//text label for the x axis
	this.xAxis(this.canvas.select(".xAxis"));
	this.canvas.select("#" + this.widgetID + "_labelXAxis")
	    .attr("x",(canvasWidth/2.0))
	    .attr("y",(canvasHeight + this.margins.top + 20))
	    .style("text-anchor", "middle")
	    .text(this.xLabel);
	//text label for the y axis
	this.yAxis.tickFormat(d3.format(".0s"));
	this.yAxis(this.canvas.select(".yAxis"));
	this.canvas.select("#" + this.widgetID + "_labelYAxis")
	    .attr("transform", "rotate(-90)")
	    .attr("y", 0 - this.margins.left)
	    .attr("x",0 - (canvasHeight / 2))
	    .attr("dy", "1em")
	    .style("text-anchor", "middle")
	    .text(this.yLabel);
    }

    setData(newData,keys,colors){
	//
	var range = []; 
	var domain = [];
	var classes = [];
	newData.forEach(function(d,i){
	    domain.push(i);
	    range.push(d.color);
	    classes.push(d.label);
	});
	this.colors = colors;
	//
	this.keys = classes;
	this.stack = d3.stack()
	    .keys(keys)
	    .order(d3.stackOrderNone)
	    .offset(d3.stackOffsetNone);
	//
	this.layers = this.stack(newData);
	this.xScale.domain(classes);
	this.yScale.domain([0, d3.max(this.layers[this.layers.length - 1], function(d) { return 1.05*d[1]; }) ]);
	//
	this.updatePlot();
    }
    
    updateBars(){
	if(this.stack == undefined)
	    return;
	
	var series = this.layers; 

	var layers = this.canvas
	    .selectAll(".layer")
	    .data(series);
	
	layers.exit().remove();

	layers.enter()
	    .append("g")
	    .merge(layers)
	    .attr("class","layer")
	    .style("fill", (function(d, i) {return this.colors[i]; }).bind(this));

	var layers = this.canvas
	    .selectAll(".layer")
	    .data(series);
	
	var rects = layers.selectAll("rect")
	    .data(function(d) { return d; });
	//
	rects.exit().remove();
	//
	rects.enter().append("rect")
	    .merge(rects)
	    .attr("x", (function(d,i) {return this.xScale(this.keys[i]); }).bind(this))
	    .attr("y", (function(d) { return this.yScale(d[1]); }).bind(this))
	    .attr("height", (function(d) { return this.yScale(d[0]) - this.yScale(d[1]); }).bind(this))
	    .attr("width", this.xScale.bandwidth());
    }
    
    updatePlot(){
	this.updateAxis();
	this.updateBars();
    }
}

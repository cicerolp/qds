class BoxPlot{

    constructor(container,widgetID,screenX,screenY,totalWidth,totalHeight){
	//set margins
	this.renderingArea = {x:screenX,y:screenY,width:totalWidth,height:totalHeight};
	this.margins = {left:10,right:10,top:10,bottom:30}
	this.canvasWidth = this.renderingArea.width - this.margins.left - this.margins.right;
	this.canvasHeight = this.renderingArea.height - this.margins.top - this.margins.bottom;
	this.widgetID = widgetID;

	//
	this.canvas = container
	    .append("g")
	    .attr("id",widgetID + "_group_plot" )
	    .attr("transform","translate("+(this.renderingArea.x+this.margins.left) + ", " + (this.renderingArea.y+this.margins.top) + ")");
	
	//
	this.xScale = d3.scaleBand()
	    .range([0,this.canvasWidth])
	    .paddingInner([0.3])
	    .paddingOuter([0.4]);
	this.xAxis  = d3.axisBottom(this.xScale);
	this.canvas
	    .append("g")
	    .attr("class","xAxis")
	    .attr("transform","translate(0," + (5+this.canvasHeight)  + ")");

	//
	this.yScale = d3.scaleLinear().range([this.canvasHeight,0]);
	this.yAxis  = d3.axisLeft(this.yScale);
	this.canvas
	    .append("g")
	    .attr("class","yAxis")
	    .attr("transform","translate("+(this.margins.left+15) + ",0)");

	//
	var widget = this;
	this.transform = d3.zoomTransform(container);
	var zoom = d3.zoom()
	    .on("zoom", function(){
		widget.zoomed(widget,d3.event);
	    });

	container.call(zoom);

	
	//
	this.canvas.append("text").attr("id",widgetID + "_labelXAxis");
	this.canvas.append("text").attr("id",widgetID + "_labelYAxis");
	this.xLabel = "";
	this.yLabel = "";
	//
	this.updatePlot();
    }

    zoomed(widget,event){
	widget.transform = event.transform;
	widget.updatePlot();
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
	var yScale = this.transform.rescaleY(this.yScale);
	this.yAxis.scale(yScale);
	this.canvas.selectAll(".yAxis").call(this.yAxis);

	
	this.yAxis(this.canvas.select(".yAxis"));
	this.canvas.select("#" + this.widgetID + "_labelYAxis")
	    .attr("transform", "rotate(-90)")
	    .attr("y", 0 - this.margins.left)
	    .attr("x",0 - (canvasHeight / 2))
	    .attr("dy", "1em")
	    .style("text-anchor", "middle")
	    .text(this.yLabel);
    }

    setData(newData){
	//
	this.data = newData;//.map(d=>[d.key,d.values]);
	//
	this.xScale.domain(this.data.map(d=>d[0]));
	this.yScale.domain([0, d3.max(this.data.map(d=>d[5]))]);
	//
	this.updatePlot();
    }
    
    updateBars(){
	if(this.data == undefined)
	    return;

	//spine
	var spines = this.canvas
	    .selectAll(".spine")
	    .data(this.data);


	var yScale = this.transform.rescaleY(this.yScale);
	
	spines.exit().remove();
	spines
	    .enter()
	    .append("line")
	    .attr("class","spine")
	    .merge(spines)
	    .attr("x1",(function(d){return this.xScale(d[0])+this.xScale.bandwidth()/2}).bind(this))
	    .attr("y1",(function(d,i){return yScale(d[1]);}).bind(this))
	    .attr("x2",(function(d){return this.xScale(d[0])+this.xScale.bandwidth()/2}).bind(this))
	    .attr("y2",(function(d,i){return yScale(d[5]);}).bind(this))
	    .attr("stroke","black");

	//bodies
	var bodies = this.canvas.selectAll(".body").data(this.data);

	bodies.exit().remove();
	bodies
	    .enter()
	    .append("rect")
	    .merge(bodies)
	    .attr("class","body")
	    .attr("width",this.xScale.bandwidth())
	    .attr("x",(d=>this.xScale(d[0])).bind(this))
	    .attr("y",(d=>yScale(d[4])).bind(this))
	    .attr("height",(d=>yScale(d[2])-yScale(d[4])).bind(this))
	    .attr("stroke","black")
	    .attr("fill","white");

	//
	var medians= this.canvas.selectAll(".median").data(this.data);
	medians.exit().remove();

	medians.enter()
	    .append("line")
	    .attr("class","median")
	    .merge(medians)
	    .attr("x1",(function(d){return this.xScale(d[0])}).bind(this))
	    .attr("y1",(function(d,i){return yScale(d[3]);}).bind(this))
	    .attr("x2",(function(d){return this.xScale(d[0])+this.xScale.bandwidth()}).bind(this))
	    .attr("y2",(function(d,i){return yScale(d[3]);}).bind(this))
	    .attr("stroke","black");
	
    }
    
    updatePlot(){
	this.updateAxis();
	this.updateBars();
    }
}


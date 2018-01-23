class PlacesRowChartWidget extends BarChartWidget{

    setTextChangeCallback(f){
	this.selectionChangedCallback = f;
	this.updatePlot();
    }

    updateBars(){
	if(this.data == undefined)
	    return;

	//
	var bars = this.canvas
	    .selectAll(".bar")
	    .data(this.data);

	console.log(this.data);
	console.log(this.yScale.domain());
	
	bars.exit().remove();
	var newBars = bars.enter()
	    .append("rect")
	    .merge(bars)
	    .attr("class","bar")
	    .attr("x", 0)
	    .attr("fill","#e5f5f9")
	    .attr("stroke","black")
	    .attr("y", (function(d) { return this.yScale(d.key); }).bind(this))
	    .attr("height", (function(d) { return this.yScale.bandwidth(); }).bind(this))
	    .attr("width", d=>this.xScale(d.values))
	    .on("mouseover", function(){ d3.select(this).attr("fill", "orange"); } )
	    .on("mouseout", function(){ d3.select(this).attr("fill", "#e5f5f9"); } )
	    .on("click",(function(d){
		if(this.selectionChangedCallback) {
		    this.selectionChangedCallback(d);
		}
	    }).bind(this));

	//labels
	var labels = this.canvas
	    .selectAll(".label")
	    .data(this.data);
	
	labels.exit().remove();
	labels.enter()
	    .append("text")
	    .merge(labels)
	    .attr("class","label")
	    .attr("x", 10)
	    .attr("y", (function(d) { return this.yScale(d.key)+this.yScale.bandwidth()/2; }).bind(this))
	    .attr("fill","black")
	    .style("font-weight","bold")
	    .style("pointer-events", "none")
	    .attr("alignment-baseline","middle")
	    .text(d=>d.label);	
    }

    updatePlot(){
	super.updateAxis();
	this.updateBars();
    }
}

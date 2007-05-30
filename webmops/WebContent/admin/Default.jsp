<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="file list">
	<stripes:layout-component name="contents">
		<ul>
			<li><a href="Distributions.action">Available distributions</a></li>
			<li><a href="Mirrors.action">Available mirrors</a></li>
			<li><a href="AddDistribution.action">New distribution</a></li>
			<li><a href="AddMirror.action">New mirror</a></li>
		</ul>
	</stripes:layout-component>
</stripes:layout-render>
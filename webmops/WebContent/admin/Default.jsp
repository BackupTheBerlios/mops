<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="Admin ">
	<stripes:layout-component name="contents">
		<ul>
			<li><stripes:link beanclass="ru.rpunet.webmops.web.admin.DashboardActionBean">Dashboard</stripes:link></li>
			<li><a href="Distributions.action">Available distributions</a></li>
			<li><a href="Mirrors.action">Available mirrors</a></li>
			<li><a href="AddDistribution.action">New distribution</a></li>
			<li><a href="AddMirror.action">New mirror</a></li>
		</ul>
	</stripes:layout-component>
</stripes:layout-render>
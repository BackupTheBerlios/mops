<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="Add distribution">
	<stripes:layout-component name="contents">
		<h2>Add new distribution:</h2>
		<stripes:form beanclass="ru.rpunet.webmops.web.admin.AddDistributionActionBean" method="get">
		<div id="form-errors">
		<stripes:errors />
		</div><br />
			Name: <stripes:text name="distr.name" value="" /><br />
			Description: <stripes:text name="distr.description" value="" /><br />
			<stripes:submit name="quickSave" value="Save" />
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>
<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="Add mirror">
	<stripes:layout-component name="contents">
		<h2>Add mirror </h2>
		<stripes:form beanclass="ru.rpunet.webmops.web.admin.AddMirrorActionBean">
		<stripes:errors /><br />
		Name: <stripes:text name="mirror.name" /><br/>
		URL: <stripes:text name="mirror.url" /><br />
		Type: 
		<stripes:select name="mirror.type" value="HTTP">
			<stripes:options-enumeration enum="ru.rpunet.webmops.model.MirrorType" />
		</stripes:select><br />
		Distribution:
		<stripes:select name="distributionId">
			<stripes:options-collection collection="${actionBean.distributions}" value="id" label="name" />
		</stripes:select>
		
		<stripes:submit name="addMirror" value="Add" />
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>
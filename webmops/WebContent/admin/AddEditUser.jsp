<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="add user">
	<stripes:layout-component name="contents">
		<h2>User manager</h2><br />
		<stripes:form beanclass="ru.rpunet.webmops.web.admin.AddEditUserActionBean">
			<stripes:errors /><br />
			<stripes:label for="user.firstName" />: <stripes:text name="user.firstName" /><br />
			<stripes:label for="user.lastName" />: <stripes:text name="user.lastName" /><br />
			<stripes:label for="user.email" />: <stripes:text name="user.email" /><br />
			<stripes:label for="user.login" />: <stripes:text name="user.login" /><br />
			<stripes:label for="user.password" />: <stripes:password name="user.password" /><br />
			<stripes:label for="confirmPassword" />: <stripes:password name="confirmPassword" /><br />
			<stripes:label for="user.group" />: 
			<stripes:select name="user.group" value="USER">
					<stripes:options-enumeration enum="ru.rpunet.webmops.model.PersonGroups" />
			</stripes:select><br />
			<stripes:submit name="saveUser" value="Save user" />
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>
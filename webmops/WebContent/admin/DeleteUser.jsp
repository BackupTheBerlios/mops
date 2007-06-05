<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="Delete user">
	<stripes:layout-component name="contents">
		<h2>Really delete user ${actionBean.user.firstName} (${actionBean.user.login})?</h2><br />
		<stripes:form beanclass="ru.rpunet.webmops.web.DeleteUserActionBean">
			<stripes:hidden name="personId" value="${actionBean.user.id}" />
			<stripes:submit name="deleteUser" value="Yes" />  
			<stripes:link beanclass="ru.rpunet.webmops.web.admin.DashboardActionBean">No</stripes:link>
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>
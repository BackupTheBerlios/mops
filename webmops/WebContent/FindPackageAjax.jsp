<%@ taglib prefix="c" uri=<http://java.sun.com/jsp/jstl/core> %>
<ul>
	<c:forEach var="pkg" items="${ actionBean.packages}">
		<li id="${pkg.id}">${pkg.name}</li>
	</c:forEach>
</ul>
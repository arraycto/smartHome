package com.hr.mqtt.web;

import com.hr.mqtt.dao.impl.QueryDataDao;

import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.List;

/**
 * @author LDC
 * @create 2020-12-24 23:22
 */

public class QueryMq2Servlet extends HttpServlet {
    private QueryDataDao dao = new QueryDataDao();
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        doGet(request, response);
    }

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        String pagers = request.getParameter("pagers");
        //System.out.println("访问成功");
        List<Mq2> resultList = dao.queryMq2(pagers);
        System.out.println(resultList.toString());
        //System.out.println(resultList.toString());
        PrintWriter writer = response.getWriter();
        StringBuffer buffer = new StringBuffer();
        buffer.append("[");
        for (Mq2 mq2 : resultList) {
            buffer.append("{\"mq2\":\"").append(mq2.getMq2()).append("\"},");
        }
        //去掉最后一个逗号
        buffer.deleteCharAt(buffer.length() - 1);
        buffer.append("]");
        writer.println(buffer);
    }
}
